//
//  ChordRecognizer.hpp
//  CSC475-pitch_effect-analyzer
//
//  Created by Owen on 2026-03-8.
//  Copyright © 2026 UVIC CSC475. All rights reserved.
//

#pragma once
#include <array>
#include <vector>
#include <string>
#include <JuceHeader.h>

using ChromaVector = std::array<float, 12>;

struct ChordResult
{
    std::string  chordName  = "N/C";
    int          rootNote   = -1;
    ChromaVector chroma     = {};
    float        confidence = 0.f;
};

struct ChordTemplate
{
    std::string  name;
    int          rootNote;
    ChromaVector profile = {};
};

class ChordRecognizer
{
public:
    ChordRecognizer(double sampleRate, int fftSize);

    // Call from audio thread every FFT frame
    void update(const std::vector<float>& fftMagnitudes);

    // Call from GUI thread to get latest result
    bool popResult(ChordResult& result);

private:
    double sampleRate;
    int    fftSize;

    std::vector<ChordTemplate> templates;

    static constexpr int queueSize = 32;
    juce::AbstractFifo   fifo      { queueSize };
    ChordResult          queue[queueSize];

    void         pushToQueue      (const ChordResult& result);
    ChromaVector extractChroma    (const std::vector<float>& fftMagnitudes);
    float        cosineSimilarity (const ChromaVector& a, const ChromaVector& b);
    void         buildTemplates   ();
};
