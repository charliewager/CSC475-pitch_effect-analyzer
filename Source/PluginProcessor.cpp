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
    // initialize param pointers
    rate = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("rate"));
    // assert param is not null
    jassert(rate != nullptr);

    depth = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("depth"));
    // assert param is not null
    jassert(depth != nullptr);

    feedback = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("feedback"));
    // assert param is not null
    jassert(feedback != nullptr);

    effect = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("effect"));
    // assert param is not null
    jassert(effect != nullptr);
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
    //juce::dsp::ProcessSpec spec = juce::dsp::ProcessSpec{ sampleRate, (uint32)samplesPerBlock, getTotalNumOutputChannels() };
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;
    for (auto& c : chorusVoices)
        c.prepare(spec);
    
    // Initial global settings (centre delay & mix defaults)
    for (auto& c : chorusVoices)
    {
        c.setCentreDelay(7.5f);      // ms, classic chorus region[web:7]
        c.setMix(0.5f);         // 0–1
    }

    int numChannels = getTotalNumOutputChannels();
    dryBuffer.setSize(numChannels, samplesPerBlock);
    voiceBuffer.setSize(numChannels, samplesPerBlock);
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

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    float r = rate->get();
    float d = depth->get();
    float f = feedback->get();
    int effectIndex = effect->getIndex();

    if (effectIndex == 0) // Chorus
    {
        chorusVoices[0].setRate(r);
        chorusVoices[0].setDepth(d);
        chorusVoices[0].setFeedback(f);
        chorusVoices[0].setMix(0.5f);
        chorusVoices[0].setCentreDelay(7.5f);

        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        chorusVoices[0].process(context);
    }
    else if (effectIndex == 1) // Multi-Voice Chorus
    {
        const float baseDelayMs    = 7.5f;
        const float minDelayMs     = 1.0f;
        const float maxDelayMs     = 20.0f;
        const float delaySpreadMs  = 2.0f;
        const float depthSpread    = 0.15f;
        const float rateSpreadHz   = 0.1f;
        const float feedbackSpread = 0.1f;

        for (int i = 0; i < numVoices; ++i)
        {
            const float t = (numVoices > 1) ? (static_cast<float>(i) / (numVoices - 1)) : 0.0f;
            const float u = 2.0f * t - 1.0f; // map 0..1 -> -1..1

            float voiceDelay    = juce::jlimit(minDelayMs, maxDelayMs, baseDelayMs + u * delaySpreadMs);
            float voiceDepth    = juce::jlimit(0.0f, 1.0f, d * (1.0f + u * depthSpread));
            float voiceRate     = std::max(0.01f, r + u * rateSpreadHz);
            float voiceFeedback = juce::jlimit(-1.0f, 1.0f, f + u * feedbackSpread);

            chorusVoices[i].setRate(voiceRate);
            chorusVoices[i].setDepth(voiceDepth);
            chorusVoices[i].setFeedback(voiceFeedback);
            chorusVoices[i].setCentreDelay(voiceDelay);
            chorusVoices[i].setMix(1.0f); // fully wet — dry/wet blend handled manually below
        }

        const int numSamples  = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();

        // Save dry input
        for (int ch = 0; ch < numChannels; ++ch)
            dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);

        // Process voice 0 in-place on buffer (becomes first voice's output)
        {
            auto block   = juce::dsp::AudioBlock<float>(buffer);
            auto context = juce::dsp::ProcessContextReplacing<float>(block);
            chorusVoices[0].process(context);
        }

        // Process each remaining voice from the dry input, then add to buffer
        for (int v = 1; v < numVoices; ++v)
        {
            for (int ch = 0; ch < numChannels; ++ch)
                voiceBuffer.copyFrom(ch, 0, dryBuffer, ch, 0, numSamples);

            auto block   = juce::dsp::AudioBlock<float>(voiceBuffer.getArrayOfWritePointers(),
                                                        (size_t)numChannels, (size_t)numSamples);
            auto context = juce::dsp::ProcessContextReplacing<float>(block);
            chorusVoices[v].process(context);

            for (int ch = 0; ch < numChannels; ++ch)
                buffer.addFrom(ch, 0, voiceBuffer, ch, 0, numSamples);
        }

        // Normalize the summed wet voices, then blend 50/50 with dry
        buffer.applyGain(1.0f / static_cast<float>(numVoices));
        for (int ch = 0; ch < numChannels; ++ch)
            buffer.addFrom(ch, 0, dryBuffer, ch, 0, numSamples);
        buffer.applyGain(0.5f);
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
    juce::MemoryOutputStream mem_out(destData, true);
    apvts.state.writeToStream(mem_out);
}

void CSC475pitch_effectanalyzerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout CSC475pitch_effectanalyzerAudioProcessor::createParameterLayout() {
    // initialize param obj
    APVTS::ParameterLayout param_layout;
    // use namespace to eliminate the need to type juce::
    using namespace juce;

    // add parameters
    param_layout.add(std::make_unique<AudioParameterFloat>("rate", "Rate", NormalisableRange<float>(0.1f, 2.0f, 0.001f, 1.0f), 0.1));
    param_layout.add(std::make_unique<AudioParameterFloat>("depth", "Depth", NormalisableRange<float>(0.0f, 1.0f, 0.001f, 1.0f), 0.25));
    param_layout.add(std::make_unique<AudioParameterFloat>("feedback", "Feedback", NormalisableRange<float>(-0.25f, 0.25f, 0.001f, 1.0f), 0));
    param_layout.add(std::make_unique<AudioParameterChoice>("effect", "Effect Type Choice", StringArray("Chorus", "Multi-Voice Chorus"), 0));

    // return layout
    return param_layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CSC475pitch_effectanalyzerAudioProcessor();
}
