/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CSC475pitch_effectanalyzerAudioProcessor::CSC475pitch_effectanalyzerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

CSC475pitch_effectanalyzerAudioProcessor::~CSC475pitch_effectanalyzerAudioProcessor()
{
}

//==============================================================================
const juce::String CSC475pitch_effectanalyzerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CSC475pitch_effectanalyzerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CSC475pitch_effectanalyzerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CSC475pitch_effectanalyzerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CSC475pitch_effectanalyzerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CSC475pitch_effectanalyzerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CSC475pitch_effectanalyzerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CSC475pitch_effectanalyzerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CSC475pitch_effectanalyzerAudioProcessor::getProgramName (int index)
{
    return {};
}

void CSC475pitch_effectanalyzerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CSC475pitch_effectanalyzerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void CSC475pitch_effectanalyzerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CSC475pitch_effectanalyzerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CSC475pitch_effectanalyzerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (totalNumInputChannels > 0)
    {
        auto* x = buffer.getReadPointer(0);
        auto n = buffer.getNumSamples();

        double sumSquares = 0.0;
        float peak = 0.0f;
        for (int i = 0; i < n; ++i){
            const float s = x[i];
            sumSquares += (double) s * (double) s;
            peak = juce::jmax(peak, std::abs(x[i]));

            fifo[(size_t) fifoIndex++] = s;

            if (fifoIndex == fftSize){
                fifoIndex = 0;
                std::fill(fftData.begin(), fftData.end(), 0.0f);
                for (int j=0; j < fftSize; ++j){
                    fftData[(size_t) j] = fifo[(size_t) j];
                }

                window.multiplyWithWindowingTable(fftData.data(), fftSize);

                fft.performFrequencyOnlyForwardTransform(fftData.data());

                magsVersion.fetch_add(1, std::memory_order_acq_rel);

                for (int k = 0; k < fftSize / 2; ++k){
                    magnitudes[(size_t) k] = fftData[(size_t) k];
                }

                magsVersion.fetch_add(1, std::memory_order_acq_rel);
            }
        }

        auto rms = std::sqrt(sumSquares / (double) n);
        inputRms.store((float) rms, std::memory_order_relaxed);
    }
    else 
    {
        inputRms.store(0.0f, std::memory_order_relaxed);
    }
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            channelData[i] *= 0.0f;
        
    }

    if (totalNumOutputChannels > 0){
        const float* y = buffer.getReadPointer(0);
        const int n = buffer.getNumSamples();

        for (int i = 0; i < n; ++i){
            outputFifo[(size_t) outputFifoIndex++] = y[i];

            if (outputFifoIndex == fftSize){
                outputFifoIndex = 0;
                std::fill(outputFftData.begin() , outputFftData.end(), 0.0f);

                for (int j = 0; j<fftSize; ++j){
                    outputFftData[(size_t) j] = outputFifo[(size_t) j];
                }
                window.multiplyWithWindowingTable(outputFftData.data(), fftSize);
                fft.performFrequencyOnlyForwardTransform(outputFftData.data());
                outputMagsVersion.fetch_add(1, std::memory_order_acq_rel);

                for (int k = 0; k<fftSize/2; ++k){
                    outputMagnitudes[(size_t) k] = outputFftData[(size_t) k];
                }
                outputMagsVersion.fetch_add(1, std::memory_order_acq_rel);
            }
        }
    }

}

//==============================================================================
bool CSC475pitch_effectanalyzerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CSC475pitch_effectanalyzerAudioProcessor::createEditor()
{
    return new CSC475pitch_effectanalyzerAudioProcessorEditor (*this);
}

//==============================================================================
void CSC475pitch_effectanalyzerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CSC475pitch_effectanalyzerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CSC475pitch_effectanalyzerAudioProcessor();
}

bool CSC475pitch_effectanalyzerAudioProcessor::getLatestMagnitudes(std::array<float, fftSize/2> & dest) const
{
    for (int tries = 0; tries <3; ++tries){
        auto v1 = magsVersion.load(std::memory_order_acquire);
        if (v1 & 1u){
            continue;
        }

        dest = magnitudes;

        auto v2 = magsVersion.load(std::memory_order_acquire);
        if (v1 == v2 && !(v2 & 1u)){
            return true;
        }
    }
    return false;
}

bool CSC475pitch_effectanalyzerAudioProcessor::getLatestOutputMagnitudes(std::array<float, fftSize/2> & dest) const
{
    for (int tries = 0; tries <3; ++tries){
        auto v1 = outputMagsVersion.load(std::memory_order_acquire);
        if (v1 & 1u){
            continue;
        }

        dest = outputMagnitudes;

        auto v2 = outputMagsVersion.load(std::memory_order_acquire);
        if (v1 == v2 && !(v2 & 1u)){
            return true;
        }
    }
    return false;
}