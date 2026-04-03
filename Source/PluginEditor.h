/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SpectrumComponent.h"
#include "SpectrogramComponent.h"
#include "ChordRecognizer.hpp"


//==============================================================================
/**
*/
class CSC475pitch_effectanalyzerAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    CSC475pitch_effectanalyzerAudioProcessorEditor (CSC475pitch_effectanalyzerAudioProcessor&);
    ~CSC475pitch_effectanalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CSC475pitch_effectanalyzerAudioProcessor& audioProcessor;
    SpectrumComponent inputSpectrum;
    SpectrumComponent outputSpectrum;
    SpectrogramComponent inputSpectrogram;
    void timerCallback() override;

    juce::Label inputChordLabel;
    juce::Label outputChordLabel;

    // knobs and attachments 
    juce::Slider rate_knob{ juce::Slider::SliderStyle::Rotary, juce::Slider::TextEntryBoxPosition::TextBoxBelow };
    juce::Label r_label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateAttachment;

    juce::Slider depth_knob{ juce::Slider::SliderStyle::Rotary, juce::Slider::TextEntryBoxPosition::TextBoxBelow };
    juce::Label d_label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment;

    juce::Slider feedback_knob{ juce::Slider::SliderStyle::Rotary, juce::Slider::TextEntryBoxPosition::TextBoxBelow };
    juce::Label f_label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;

    juce::ComboBox effect;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> effectAttachment;

    double lastSampleRate { 0.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CSC475pitch_effectanalyzerAudioProcessorEditor)
};
