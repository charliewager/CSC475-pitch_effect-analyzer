#pragma once
#include <JuceHeader.h>

class SpectrogramComponent : public juce::Component
{
public:
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int numBins = fftSize / 2;

    SpectrogramComponent() : spectrogramImage(juce::Image::RGB, 512, numBins, true)
    {
    }

    void pushMagnitudes(const std::array<float, numBins> &mags)
    {
        spectrogramImage.moveImageSection(
            0, 0,
            1, 0,
            spectrogramImage.getWidth() - 1,
            spectrogramImage.getHeight());

        const int rightX = spectrogramImage.getWidth() - 1;

        for (int bin = 0; bin < numBins; ++bin)
        {
            float m = mags[(size_t)bin];

            float db = juce::Decibels::gainToDecibels(m + 1.0e-6f);
            float norm = juce::jmap(db, -80.0f, 0.0f, 0.0f, 1.0f);
            norm = juce::jlimit(0.0f, 1.0f, norm);

            juce::Colour colour;

            if (norm < 0.25f)
                colour = juce::Colours::black.interpolatedWith(juce::Colours::darkblue, norm / 0.25f);
            else if (norm < 0.5f)
                colour = juce::Colours::darkblue.interpolatedWith(juce::Colours::purple, (norm - 0.25f) / 0.25f);
            else if (norm < 0.75f)
                colour = juce::Colours::purple.interpolatedWith(juce::Colours::orange, (norm - 0.5f) / 0.25f);
            else
                colour = juce::Colours::orange.interpolatedWith(juce::Colours::yellow, (norm - 0.75f) / 0.25f);

            // flip vertically so low frequencies are at the bottom
            int y = numBins - 1 - bin;
            spectrogramImage.setPixelAt(rightX, y, colour);
        }
        repaint();
    }

    void paint(juce::Graphics &g) override
    {
        g.fillAll(juce::Colours::black);

        g.drawImageWithin(
            spectrogramImage,
            0, 0,
            getWidth(), getHeight(),
            juce::RectanglePlacement::stretchToFit);

        g.setColour(juce::Colours::white);
        g.drawRect(getLocalBounds(), 1);
    }

private:
    juce::Image spectrogramImage;
};