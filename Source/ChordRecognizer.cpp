//
//  ChordRecognizer.cpp
//  CSC475-pitch_effect-analyzer
//
//  Created by Owen on 2026-03-8.
//  Copyright © 2026 UVIC CSC475. All rights reserved.
//

#include "ChordRecognizer.hpp"

#include <cmath>
#include <numeric>
#include <algorithm>

namespace
{
    constexpr float kMinFrequencyHz        = 32.7f;   // roughly C1
    constexpr float kMaxFrequencyHz        = 5000.0f; // ignore very high bins that add noise
    constexpr float kChromaEmaAlpha        = 0.82f;   // higher = smoother
    constexpr float kAcceptThreshold       = 0.56f;
    constexpr float kStrongThreshold       = 0.68f;
    constexpr float kMinMarginToSwitch     = 0.06f;
    constexpr int   kFramesToSwitch        = 3;
    constexpr int   kFramesToDropToNoChord = 6;

    inline float clamp01(float value)
    {
        return std::max(0.0f, std::min(1.0f, value));
    }
}

ChordRecognizer::ChordRecognizer(double sampleRate, int fftSize)
    : sampleRate(sampleRate), fftSize(fftSize)
{
    buildTemplates();
}

void ChordRecognizer::update(const std::vector<float>& fftMagnitudes)
{
    ChordResult result;
    ChromaVector frameChroma = extractChroma(fftMagnitudes);

    if (!hasSmoothedFrame)
    {
        smoothedChroma = frameChroma;
        hasSmoothedFrame = true;
    }
    else
    {
        for (int i = 0; i < 12; ++i)
            smoothedChroma[i] = kChromaEmaAlpha * smoothedChroma[i]
                              + (1.0f - kChromaEmaAlpha) * frameChroma[i];
    }

    smoothChromaInPlace(smoothedChroma);
    result.chroma = smoothedChroma;

    float bestScore = -1.0f;
    float secondBestScore = -1.0f;
    int bestIndex = -1;

    for (int i = 0; i < static_cast<int>(templates.size()); ++i)
    {
        const float score = scoreTemplate(smoothedChroma, templates[i]);

        if (score > bestScore)
        {
            secondBestScore = bestScore;
            bestScore = score;
            bestIndex = i;
        }
        else if (score > secondBestScore)
        {
            secondBestScore = score;
        }
    }

    const float margin = bestScore - secondBestScore;
    const bool candidateIsReliable = bestIndex >= 0
                                  && bestScore >= kAcceptThreshold
                                  && margin >= kMinMarginToSwitch;

    if (candidateIsReliable)
    {
        uncertainFrames = 0;

        if (bestIndex == stableTemplateIndex)
        {
            pendingTemplateIndex = -1;
            pendingFrames = 0;
            stableConfidence = std::max(stableConfidence, bestScore);
            stableConfidence = 0.75f * stableConfidence + 0.25f * bestScore;
        }
        else
        {
            if (bestIndex != pendingTemplateIndex)
            {
                pendingTemplateIndex = bestIndex;
                pendingFrames = 1;
            }
            else
            {
                ++pendingFrames;
            }

            const bool promoteImmediately = stableTemplateIndex < 0 && bestScore >= kStrongThreshold;
            if (promoteImmediately || pendingFrames >= kFramesToSwitch)
            {
                stableTemplateIndex = bestIndex;
                stableConfidence = bestScore;
                pendingTemplateIndex = -1;
                pendingFrames = 0;
            }
        }
    }
    else
    {
        pendingTemplateIndex = -1;
        pendingFrames = 0;
        ++uncertainFrames;

        if (stableTemplateIndex >= 0)
            stableConfidence *= 0.92f;

        if (uncertainFrames >= kFramesToDropToNoChord)
        {
            stableTemplateIndex = -1;
            stableConfidence = 0.0f;
        }
    }

    if (stableTemplateIndex >= 0)
    {
        result.chordName  = templates[stableTemplateIndex].name;
        result.rootNote   = templates[stableTemplateIndex].rootNote;
        result.confidence = clamp01(stableConfidence);
    }
    else
    {
        result.chordName  = "N/C";
        result.rootNote   = -1;
        result.confidence = 0.0f;
    }

    pushToQueue(result);
}

bool ChordRecognizer::popResult(ChordResult& result)
{
    int start1, size1, start2, size2;
    fifo.prepareToRead(1, start1, size1, start2, size2);
    if (size1 == 0) return false;
    result = queue[start1];
    fifo.finishedRead(size1);
    return true;
}

void ChordRecognizer::pushToQueue(const ChordResult& result)
{
    int start1, size1, start2, size2;
    fifo.prepareToWrite(1, start1, size1, start2, size2);
    if (size1 > 0) queue[start1] = result;
    fifo.finishedWrite(size1);
}

ChromaVector ChordRecognizer::extractChroma(const std::vector<float>& fftMagnitudes)
{
    ChromaVector chroma = {};
    const int numBins = std::min(static_cast<int>(fftMagnitudes.size()), fftSize / 2);

    for (int bin = 1; bin < numBins; ++bin)
    {
        const float freq = static_cast<float>(bin) * static_cast<float>(sampleRate) / static_cast<float>(fftSize);
        if (freq < kMinFrequencyHz || freq > kMaxFrequencyHz)
            continue;

        const float midi = 69.0f + 12.0f * std::log2(freq / 440.0f);
        const int midiRounded = static_cast<int>(std::round(midi));
        int pitchClass = midiRounded % 12;
        if (pitchClass < 0)
            pitchClass += 12;

        const float mag = std::max(0.0f, fftMagnitudes[bin]);
        if (mag <= 0.0f)
            continue;

        // A log-like compression makes strong harmonics less dominant.
        const float compressedMag = std::sqrt(mag);

        // Penalize higher frequencies a bit so upper harmonics do not dominate.
        const float octaveWeight = 1.0f / (1.0f + 0.12f * std::max(0.0f, midi - 48.0f));

        // Spread some energy into neighboring pitch classes when the bin lands
        // between semitones instead of hard-assigning the whole bin to one class.
        const float pitchClassPosition = midi - 12.0f * std::floor(midi / 12.0f);
        const int lowerClass = static_cast<int>(std::floor(pitchClassPosition)) % 12;
        const int upperClass = (lowerClass + 1) % 12;
        const float frac = pitchClassPosition - std::floor(pitchClassPosition);

        const float energy = compressedMag * octaveWeight;
        chroma[lowerClass] += energy * (1.0f - frac);
        chroma[upperClass] += energy * frac;
    }

    smoothChromaInPlace(chroma);

    const float sum = std::accumulate(chroma.begin(), chroma.end(), 0.0f);
    if (sum > 0.0f)
    {
        for (auto& v : chroma)
            v /= sum;
    }

    return chroma;
}

void ChordRecognizer::smoothChromaInPlace(ChromaVector& chroma) const
{
    ChromaVector smoothed = chroma;

    for (int i = 0; i < 12; ++i)
    {
        const int prev = (i + 11) % 12;
        const int next = (i + 1) % 12;
        smoothed[i] = 0.60f * chroma[i] + 0.20f * chroma[prev] + 0.20f * chroma[next];
    }

    chroma = smoothed;
}

float ChordRecognizer::cosineSimilarity(const ChromaVector& a, const ChromaVector& b)
{
    float dot = 0.f, normA = 0.f, normB = 0.f;
    for (int i = 0; i < 12; ++i)
    {
        dot   += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    if (normA == 0.f || normB == 0.f) return 0.f;
    return dot / (std::sqrt(normA) * std::sqrt(normB));
}

float ChordRecognizer::scoreTemplate(const ChromaVector& chroma, const ChordTemplate& chord) const
{
    float inChordEnergy = 0.0f;
    float outOfChordEnergy = 0.0f;
    int chordToneCount = 0;

    for (int i = 0; i < 12; ++i)
    {
        if (chord.profile[i] > 0.0f)
        {
            inChordEnergy += chroma[i];
            ++chordToneCount;
        }
        else
        {
            outOfChordEnergy += chroma[i];
        }
    }

    if (chordToneCount == 0)
        return 0.0f;

    const float meanInChordEnergy = inChordEnergy / static_cast<float>(chordToneCount);
    const float rootEnergy = chroma[chord.rootNote];
    const float fifthEnergy = chroma[(chord.rootNote + 7) % 12];
    const float similarity = cosineSimilarity(chroma, chord.profile);

    const float score = 0.50f * similarity
                      + 0.22f * inChordEnergy
                      + 0.18f * rootEnergy
                      + 0.08f * fifthEnergy
                      + 0.10f * meanInChordEnergy
                      - 0.20f * outOfChordEnergy;

    return clamp01(score);
}

// define 9 chord types as intervals in semitones from the root
void ChordRecognizer::buildTemplates()
{
    const std::string noteNames[] = {"C","C#","D","D#","E","F",
                                      "F#","G","G#","A","A#","B"};
    const std::vector<std::pair<std::string, std::vector<int>>> shapes = {
        {"maj",  {0, 4, 7}},
        {"min",  {0, 3, 7}},
        {"dom7", {0, 4, 7, 10}},
        {"maj7", {0, 4, 7, 11}},
        {"min7", {0, 3, 7, 10}},
        {"dim",  {0, 3, 6}},
        {"aug",  {0, 4, 8}},
        {"sus2", {0, 2, 7}},
        {"sus4", {0, 5, 7}},
    };

    templates.clear();
    templates.reserve(12 * static_cast<int>(shapes.size()));

    for (int root = 0; root < 12; ++root)
    {
        for (const auto& [suffix, intervals] : shapes)
        {
            ChordTemplate t;
            t.name     = noteNames[root] + suffix;
            t.rootNote = root;
            t.profile.fill(0.f);

            for (int interval : intervals)
                t.profile[(root + interval) % 12] = 1.f;

            // Weighted templates help distinguish chord function better.
            t.profile[root] *= 1.25f;
            t.profile[(root + 7) % 12] += 0.10f; // small fifth emphasis when present

            const float sum = std::accumulate(t.profile.begin(), t.profile.end(), 0.f);
            if (sum > 0.0f)
            {
                for (auto& v : t.profile)
                    v /= sum;
            }

            templates.push_back(t);
        }
    }
}
