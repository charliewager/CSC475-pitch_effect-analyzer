/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class CSC475pitch_effectanalyzerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    CSC475pitch_effectanalyzerAudioProcessor();
    ~CSC475pitch_effectanalyzerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    static APVTS::ParameterLayout createParameterLayout();

    APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:
    
    // chorus things
    static constexpr int numVoices = 6;
    juce::dsp::Chorus<float> chorusVoices[numVoices];

    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> voiceBuffer;

    // ring mod stuff
    float rmPhase { 0.0f };
    float rmFeedbackState[2] { 0.0f, 0.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> rmSmoothedCarrierHz;
    float rmDCBlockX[2] { 0.0f, 0.0f };   // DC blocker: previous input per channel
    float rmDCBlockY[2] { 0.0f, 0.0f };   // DC blocker: previous output per channel
    juce::dsp::Oversampling<float> rmOversampling { 2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };

    //general things
    juce::AudioParameterFloat* rate{ nullptr };
    juce::AudioParameterFloat* depth{ nullptr };
    juce::AudioParameterFloat* feedback{ nullptr };
    juce::AudioParameterChoice* effect{ nullptr };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CSC475pitch_effectanalyzerAudioProcessor)
};
