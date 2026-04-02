#pragma once
#include <JuceHeader.h>

class SpectrumComponent : public juce::Component
{
public:
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << 10;
    static constexpr int numBins = fftSize/2;

    void setMagnitudes(const std::array<float, numBins>& newMags){
        mags = newMags;
        repaint();
    }
    void setLevel(float newLevel)
    {
        level.store(newLevel, std::memory_order_relaxed);
        repaint();
    }

    void setSampleRate(double sr) { sampleRate = sr; }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        const int width  = getWidth();
        const int height = getHeight();
        if (width <= 0 || height <= 0) return;

        const int leftMargin = 38;
        const int drawW = width - leftMargin;

        g.setColour(juce::Colours::white);
        g.drawRect(getLocalBounds().toFloat(), 1.0f);

        const int numBars = 128;
        const float barH = (float)height / (float)numBars;

        // Logarithmic frequency mapping: 20 Hz at bottom, 30000 Hz at top
        const float minFreq = 20.0f;
        const float maxFreq = 30000.0f;
        const float logRange = std::log(maxFreq / minFreq);

        // Normalisation: performFrequencyOnlyForwardTransform output peaks at
        // fftSize/2 for a full-scale sine, so divide to bring that to 1.0 (0 dB).
        const float normFactor = 2.0f / (float)fftSize;

        g.setColour(juce::Colours::limegreen);

        for (int b = 0; b < numBars; ++b)
        {
            // Frequency range this bar represents on the log scale
            float freqLo = minFreq * std::exp(logRange * (float)b       / (float)(numBars - 1));
            float freqHi = minFreq * std::exp(logRange * (float)(b + 1) / (float)(numBars - 1));

            float binLo = freqLo * (float)fftSize / (float)sampleRate;
            float binHi = freqHi * (float)fftSize / (float)sampleRate;
            binLo = juce::jlimit(0.0f, (float)(numBins - 1), binLo);
            binHi = juce::jlimit(0.0f, (float)(numBins - 1), binHi);

            int bin0 = (int)binLo;
            int bin1 = (int)binHi;

            float m = 0.0f;
            if (bin1 > bin0)
            {
                // Bar spans multiple bins — take the peak across the range
                for (int i = bin0; i <= bin1; ++i)
                    m = juce::jmax(m, mags[(size_t)i]);
            }
            else
            {
                // Sub-bin resolution (low frequencies) — interpolate
                float frac    = binLo - (float)bin0;
                int   binNext = juce::jmin(bin0 + 1, numBins - 1);
                m = mags[(size_t)bin0] * (1.0f - frac) + mags[(size_t)binNext] * frac;
            }

            // Normalize to 0–1 range then convert to dB
            m *= normFactor;
            float db   = juce::Decibels::gainToDecibels(m + 1.0e-6f);
            float norm = juce::jlimit(0.0f, 1.0f, juce::jmap(db, -80.0f, 0.0f, 0.0f, 1.0f));
            float barW = norm * (float)drawW;

            // Flipped: bar 0 (lowest freq) at the bottom
            float y = (float)(numBars - 1 - b) * barH;

            g.fillRect((float)leftMargin, y,
                       juce::jmax(1.0f, barW),
                       juce::jmax(1.0f, barH - 1.0f));
        }

        // Frequency axis marks — 1-2-5 decade series, evenly spaced on the log axis
        struct FreqMark { float hz; const char* label; };
        static constexpr FreqMark marks[] = {
            {    20.f, "20"  },
            {    50.f, "50"  },
            {   100.f, "100" },
            {   200.f, "200" },
            {   500.f, "500" },
            {  1000.f, "1k"  },
            {  2000.f, "2k"  },
            {  5000.f, "5k"  },
            { 10000.f, "10k" },
            { 20000.f, "20k" },
        };

        g.setFont(9.0f);

        for (auto& mark : marks)
        {
            if (mark.hz < minFreq || mark.hz > maxFreq) continue;

            // Y position using the same log mapping as the bars
            float barPos = std::log(mark.hz / minFreq) / logRange * (float)(numBars - 1);
            float y = ((float)(numBars - 1) - barPos) * barH + barH * 0.5f;

            g.setColour(juce::Colours::grey);
            g.drawHorizontalLine((int)y, (float)leftMargin - 4.0f, (float)leftMargin);

            g.setColour(juce::Colours::white);
            g.drawText(mark.label,
                       0, (int)(y - 5.0f), leftMargin - 5, 11,
                       juce::Justification::centredRight);
        }
    }

private:
    double sampleRate { 44100.0 };
    std::atomic<float> level { 0.0f };
    std::array<float, numBins> mags{};
};


