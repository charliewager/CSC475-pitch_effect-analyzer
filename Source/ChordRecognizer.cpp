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

ChordRecognizer::ChordRecognizer(double sampleRate, int fftSize)
    : sampleRate(sampleRate), fftSize(fftSize)
{
    buildTemplates();
}

void ChordRecognizer::update(const std::vector<float>& fftMagnitudes)
{
    ChordResult result;
    result.chroma = extractChroma(fftMagnitudes); // get chroma from fft data

    float bestScore = -1.f;
    int   bestIndex = -1;

    for (int i = 0; i < (int)templates.size(); ++i)// compare against templates
    {
        float score = cosineSimilarity(result.chroma, templates[i].profile);
        if (score > bestScore)
        {
            bestScore = score;
            bestIndex = i;
        }
    }

    if (bestIndex >= 0 && bestScore > 0.75f)// confidence level
    {
        result.chordName  = templates[bestIndex].name;
        result.rootNote   = templates[bestIndex].rootNote;
        result.confidence = bestScore;
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
    int numBins = fftSize / 2;

    for (int bin = 1; bin < numBins; ++bin)
    {
        float freq = bin * (float)sampleRate / (float)fftSize;
        if (freq < 32.7f || freq > 7902.f) continue;

        float midi      = 69.f + 12.f * std::log2(freq / 440.f);
        int midiRounded = (int)std::round(midi);
        int pitchClass  = midiRounded % 12;
        if (pitchClass < 0) pitchClass += 12;

        // EPCP: weight by harmonic importance
        // Ask "could this frequency be a harmonic of a lower fundamental?"
        // If so, downweight it so overtones don't pollute other pitch classes
        float harmonicWeight = 0.f;
        for (int h = 1; h <= 6; ++h)
        {
            float fundamental = freq / h;
            if (fundamental >= 32.7f && fundamental <= 1600.f)
                harmonicWeight += 1.f / (float)(h * h);
        }

        float mag = fftMagnitudes[bin];
        chroma[pitchClass] += mag * mag * harmonicWeight;
    }

    float maxVal = *std::max_element(chroma.begin(), chroma.end());
    if (maxVal > 0.f)
        for (auto& v : chroma) v /= maxVal;

    return chroma;
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
// define 9 chord types as intervals in semitones from the root
void ChordRecognizer::buildTemplates()
{
    const std::string noteNames[] = {"C","C#","D","D#","E","F",
                                      "F#","G","G#","A","A#","B"};
    const std::vector<std::pair<std::string, std::vector<int>>> shapes = {
        // c maj  4 semitones up and 7 semitones up  C, E, G etc.
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

    for (int root = 0; root < 12; ++root) // this is the above comments logic
        for (auto& [suffix, intervals] : shapes)
        {
            ChordTemplate t;
            t.name     = noteNames[root] + suffix;
            t.rootNote = root;
            t.profile.fill(0.f);
            for (int interval : intervals)
                t.profile[(root + interval) % 12] = 1.f; // loop through all 12 root nodes and 9 shapes to build 108 templates total
            float sum = std::accumulate(t.profile.begin(),
                                         t.profile.end(), 0.f);
            for (auto& v : t.profile) v /= sum; // normalization
            templates.push_back(t);
        }
}
