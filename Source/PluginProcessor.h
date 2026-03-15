/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ChordRecognizer.hpp"

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
    std::atomic<float> inputRms { 0.0f };

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
    
    static constexpr int fftOrder = 10; // 2^10 = 1024 point FFT
    static constexpr int fftSize = 1 << fftOrder;
    bool getLatestMagnitudes (std::array<float, fftSize / 2>& dest) const;
    bool getLatestOutputMagnitudes (std::array<float, fftSize / 2>& dest) const;
    
    std::unique_ptr<ChordRecognizer> inputChordRecognizer;
    std::unique_ptr<ChordRecognizer> outputChordRecognizer;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CSC475pitch_effectanalyzerAudioProcessor)

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann, false};

    std::array<float, fftSize> fifo {};
    int fifoIndex = 0;

    std::array<float, fftSize * 2> fftData {};

    std::array<float, fftSize / 2> magnitudes {};

    std::atomic<uint32_t> magsVersion {0};

    std::array<float, fftSize> outputFifo {};
    int outputFifoIndex = 0;

    std::array<float, fftSize*2> outputFftData {};
    std::array<float, fftSize/2> outputMagnitudes {};

    std::atomic<uint32_t> outputMagsVersion {0};

};
