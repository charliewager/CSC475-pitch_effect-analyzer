#pragma once
#include <JuceHeader.h>

class SpectrumComponent : public juce::Component
{
public:
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

        float lv = level.load(std::memory_order_relaxed);

        // Map RMS to visible range
        float norm = juce::jlimit(0.0f, 1.0f, lv * 5.0f);  
        // multiply to exaggerate small RMS values

        float height = norm * bounds.getHeight();

        g.setColour(juce::Colours::limegreen);
        g.fillRect(bounds.withY(bounds.getBottom() - height)
                          .withHeight(height));

        g.setColour(juce::Colours::white);
        g.drawText("RMS: " + juce::String(lv, 4),
                   getLocalBounds().reduced(10),
                   juce::Justification::topLeft);
    }

private:
    std::atomic<float> level { 0.0f };
};


