/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class CSC475pitch_effectanalyzerAudioProcessorEditor  : public juce::AudioProcessorEditor
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

    juce::Label input_lable;
    juce::Label output_lable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CSC475pitch_effectanalyzerAudioProcessorEditor)
};
