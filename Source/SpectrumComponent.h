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

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        auto bounds = getLocalBounds().toFloat();

        // Border
        g.setColour(juce::Colours::white);
        g.drawRect(bounds, 1.0f);

        const int width = getWidth();
        const int height = getHeight();

        if (width<=0 || height<=0) return;

        const int numBars = 128;
        const float barW = (float) width / (float) numBars;

        g.setColour(juce::Colours::limegreen);

        for (int b = 0; b < numBars; ++b){
            int bin = (b * numBins) / numBars;

            bin = juce::jlimit(0, numBins-1, bin);

            float m = mags[(size_t) bin];

            float db = juce::Decibels::gainToDecibels(m + 1.0e-6f);

            float norm = juce::jmap(db, -80.0f, 0.0f, 0.0f, 1.0f);
            norm = juce::jlimit(0.0f, 1.0f, norm);

            float barH = norm * (float) height;

            g.fillRect(b * barW, (float) height - barH, juce::jmax(1.0f, barW - 1.0f), barH);
            
        }
    }

private:
    std::atomic<float> level { 0.0f };
    std::array<float, numBins> mags{};
};


