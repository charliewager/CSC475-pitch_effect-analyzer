#pragma once
#include <JuceHeader.h>

class SpectrumComponent : public juce::Component,
                          private juce::Timer
{
public:
    SpectrumComponent() { startTimerHz(60); }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        auto r = getLocalBounds().toFloat();
        g.setColour(juce::Colours::white);
        g.drawRect(r, 1.0f);

        // placeholder line (we'll replace with FFT)
        g.drawLine(r.getX(), r.getCentreY(), r.getRight(), r.getCentreY(), 2.0f);
    }

private:
    void timerCallback() override { repaint(); }
};

