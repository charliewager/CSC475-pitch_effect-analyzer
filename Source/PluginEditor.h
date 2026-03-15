/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SpectrumComponent.h"
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
    void timerCallback() override;
    
    juce::Label inputChordLabel;
    juce::Label outputChordLabel;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CSC475pitch_effectanalyzerAudioProcessorEditor)
};
