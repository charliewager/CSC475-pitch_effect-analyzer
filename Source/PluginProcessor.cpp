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

    int numChannels = getTotalNumOutputChannels();
    dryBuffer.setSize(numChannels, samplesPerBlock);
    voiceBuffer.setSize(numChannels, samplesPerBlock);

    // Prewarm each voice with silence to stagger LFO phases evenly across one cycle.
    // Voice v is advanced by v/numVoices of a full cycle so voices are always
    // at independent points in their modulation cycle from the first sample.
    const float prewarmRate = 0.5f; // Hz — arbitrary, only determines prewarm chunk count
    for (int v = 0; v < numVoices; ++v)
    {
        chorusVoices[v].setRate(prewarmRate);
        chorusVoices[v].setDepth(0.5f);
        chorusVoices[v].setCentreDelay(7.5f);
        chorusVoices[v].setMix(1.0f);

        int samplesToProcess = static_cast<int>(
            static_cast<double>(v) * sampleRate / (prewarmRate * numVoices));

        voiceBuffer.clear();
        while (samplesToProcess > 0)
        {
            int chunkSize = std::min(samplesToProcess, samplesPerBlock);
            auto block   = juce::dsp::AudioBlock<float>(voiceBuffer.getArrayOfWritePointers(),
                                                        (size_t)numChannels, (size_t)chunkSize);
            auto context = juce::dsp::ProcessContextReplacing<float>(block);
            chorusVoices[v].process(context);
            samplesToProcess -= chunkSize;
        }
    }

    // Restore defaults (processBlock overrides params every block anyway)
    for (auto& c : chorusVoices)
    {
        c.setCentreDelay(7.5f);
        c.setMix(0.5f);
    }

    // Reset ring mod state
    rmOversampling.initProcessing(samplesPerBlock);
    const double osRate = sampleRate * rmOversampling.getOversamplingFactor();
    rmPhase = 0.0f;
    rmFeedbackState[0] = rmFeedbackState[1] = 0.0f;
    rmSmoothedCarrierHz.reset(osRate, 0.05);          // 50 ms ramp at oversampled rate
    rmSmoothedCarrierHz.setCurrentAndTargetValue(20.0f);
    rmDCBlockX[0] = rmDCBlockX[1] = 0.0f;
    rmDCBlockY[0] = rmDCBlockY[1] = 0.0f;

    // Reset harmonic ring mod state
    hrmPhase = 0.0f;
    hrmLfoPhase = 0.0f;
    hrmSmoothedCarrierHz.reset(osRate, 0.05);
    hrmSmoothedCarrierHz.setCurrentAndTargetValue(20.0f);
    hrmDCBlockX[0] = hrmDCBlockX[1] = 0.0f;
    hrmDCBlockY[0] = hrmDCBlockY[1] = 0.0f;
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
        const float delaySpreadMs  = 6.0f;  // wider spread across voices
        const float depthSpread    = 0.3f;  // wider spread across voices
        const float rateSpreadHz   = 0.4f;  // LFOs run at clearly different speeds
        const float feedbackSpread = 0.1f;

        const int numSamples  = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();

        // Save dry input before any processing
        for (int ch = 0; ch < numChannels; ++ch)
            dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);

        // Use buffer as accumulator — clear it first
        buffer.clear();

        // Accumulate the sum of per-channel pan gains to use as the normalisation divisor.
        // Left and right sums are equal by symmetry of the -1..+1 spread, so one value suffices.
        float panGainSum = 0.0f;

        for (int v = 0; v < numVoices; ++v)
        {
            const float t = (numVoices > 1) ? (static_cast<float>(v) / (numVoices - 1)) : 0.0f;
            const float u = 2.0f * t - 1.0f; // -1..+1 across voices

            float voiceDelay    = juce::jlimit(minDelayMs, maxDelayMs, baseDelayMs + u * delaySpreadMs);
            float voiceDepth    = juce::jlimit(0.0f, 1.0f, d * (1.0f + u * depthSpread));
            float voiceRate     = std::max(0.01f, r + u * rateSpreadHz);
            float voiceFeedback = juce::jlimit(-1.0f, 1.0f, f + u * feedbackSpread);

            chorusVoices[v].setRate(voiceRate);
            chorusVoices[v].setDepth(voiceDepth);
            chorusVoices[v].setFeedback(voiceFeedback);
            chorusVoices[v].setCentreDelay(voiceDelay);
            chorusVoices[v].setMix(1.0f); // fully wet — dry/wet blended manually below

            // Process this voice from the dry input
            for (int ch = 0; ch < numChannels; ++ch)
                voiceBuffer.copyFrom(ch, 0, dryBuffer, ch, 0, numSamples);

            auto block   = juce::dsp::AudioBlock<float>(voiceBuffer.getArrayOfWritePointers(),
                                                        (size_t)numChannels, (size_t)numSamples);
            auto context = juce::dsp::ProcessContextReplacing<float>(block);
            chorusVoices[v].process(context);

            // Equal-power stereo pan: voice 0 -> hard left, centre voice -> centre, last -> hard right
            if (numChannels >= 2)
            {
                const float angle     = (u + 1.0f) * (juce::MathConstants<float>::pi * 0.25f);
                const float leftGain  = std::cos(angle);
                const float rightGain = std::sin(angle);
                panGainSum += leftGain; // left and right sums are equal by symmetry
                voiceBuffer.applyGain(0, 0, numSamples, leftGain);
                voiceBuffer.applyGain(1, 0, numSamples, rightGain);
            }
            else
            {
                panGainSum += 1.0f; // no panning in mono — each voice contributes equally
            }

            // Accumulate panned voice into buffer
            for (int ch = 0; ch < numChannels; ++ch)
                buffer.addFrom(ch, 0, voiceBuffer, ch, 0, numSamples);
        }

        // Normalise by the actual pan gain sum so wet output level is independent of
        // voice count and pan spread, then blend 65% wet / 35% dry.
        const float wetLevel = 0.65f;
        buffer.applyGain(wetLevel / panGainSum);
        for (int ch = 0; ch < numChannels; ++ch)
            buffer.addFrom(ch, 0, dryBuffer, ch, 0, numSamples, 1.0f - wetLevel);

    } else if (effectIndex == 2) // Ring Modulator
    {
        const float rNorm     = (r - 0.1f) / 1.9f;
        const float carrierHz = 20.0f * std::pow(100.0f, rNorm);

        const float rmFeedbackGain = f * 2.0f;
        const float wetLevel  = d;
        const float twoPi     = juce::MathConstants<float>::twoPi;
        const double osRate   = getSampleRate() * rmOversampling.getOversamplingFactor();

        rmSmoothedCarrierHz.setTargetValue(carrierHz);

        // Upsample — osBlock points to internal oversampling memory
        auto inputBlock = juce::dsp::AudioBlock<float>(buffer);
        auto osBlock    = rmOversampling.processSamplesUp(inputBlock);

        const int osNumSamples = static_cast<int>(osBlock.getNumSamples());
        const int numChannels  = static_cast<int>(osBlock.getNumChannels());

        for (int n = 0; n < osNumSamples; ++n)
        {
            // Smoothed carrier Hz — advances ramp once per oversampled sample
            const float currentHz = rmSmoothedCarrierHz.getNextValue();
            const float carrier   = std::sin(rmPhase);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float inputSample = osBlock.getSample(ch, n);

                // Tanh saturation on feedback: smoothly bounded, warm character
                const float modInput  = inputSample + std::tanh(rmFeedbackGain * rmFeedbackState[ch]);
                const float wetSample = modInput * carrier;
                rmFeedbackState[ch]   = wetSample;

                const float outSample = (1.0f - wetLevel) * inputSample + wetLevel * wetSample;

                // DC blocker (one-pole highpass, cutoff ~3.5 Hz at 44100 Hz)
                const float dcOut = outSample - rmDCBlockX[ch] + 0.9995f * rmDCBlockY[ch];
                rmDCBlockX[ch] = outSample;
                rmDCBlockY[ch] = dcOut;

                osBlock.setSample(ch, n, dcOut);
            }

            rmPhase += static_cast<float>(twoPi * currentHz / osRate);
            if (rmPhase >= twoPi)
                rmPhase -= twoPi;
        }

        // Downsample back into buffer
        rmOversampling.processSamplesDown(inputBlock);

    } else if (effectIndex == 3) // Harmonic Ring Mod
    {
        // Rate → base carrier frequency (same exponential remap as ring mod)
        const float rNorm     = (r - 0.1f) / 1.9f;
        const float carrierHz = 20.0f * std::pow(100.0f, rNorm);

        // Feedback → LFO sweep depth: 0 = static, ±0.25 = ±50% carrier frequency sweep
        const float lfoDepth   = std::abs(f) * 2.0f;
        const float lfoRate    = 0.3f; // Hz, fixed

        const float twoPi  = juce::MathConstants<float>::twoPi;
        const double osRate = getSampleRate() * rmOversampling.getOversamplingFactor();

        hrmSmoothedCarrierHz.setTargetValue(carrierHz);

        auto inputBlock = juce::dsp::AudioBlock<float>(buffer);
        auto osBlock    = rmOversampling.processSamplesUp(inputBlock);

        const int osNumSamples  = static_cast<int>(osBlock.getNumSamples());
        const int numChannels   = static_cast<int>(osBlock.getNumChannels());
        const float lfoPhaseInc = twoPi * lfoRate / static_cast<float>(osRate);

        for (int n = 0; n < osNumSamples; ++n)
        {
            const float currentHz = hrmSmoothedCarrierHz.getNextValue();

            // LFO proportionally sweeps carrier frequency
            const float actualHz = juce::jlimit(20.0f, 2000.0f,
                currentHz * (1.0f + lfoDepth * std::sin(hrmLfoPhase)));

            // Piecewise triangle wave — odd harmonics, 1/n² rolloff, no asin needed
            auto triangleAt = [](float phase, float tp) -> float
            {
                const float t = phase / tp;
                if (t < 0.25f) return  4.0f * t;
                if (t < 0.75f) return  2.0f - 4.0f * t;
                return                 4.0f * t - 4.0f;
            };

            const float triL = triangleAt(hrmPhase, twoPi);

            // Right channel: 90° lead for stereo width without multi-carrier overhead
            float quadPhase = hrmPhase + twoPi * 0.25f;
            if (quadPhase >= twoPi) quadPhase -= twoPi;
            const float triR = triangleAt(quadPhase, twoPi);

            // Depth → AM/RM blend: d=0 pure ring mod [-1..1], d=1 pure AM [0..1]
            // modCarrier = tri * (1 - d*0.5) + d*0.5
            const float amScale  = 1.0f - d * 0.5f;
            const float amOffset = d * 0.5f;
            const float modL = triL * amScale + amOffset;
            const float modR = triR * amScale + amOffset;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float inputSample = osBlock.getSample(ch, n);
                const float outSample   = inputSample * ((ch == 0) ? modL : modR);

                // DC blocker (one-pole highpass, cutoff ~3.5 Hz at 44100 Hz)
                const float dcOut = outSample - hrmDCBlockX[ch] + 0.9995f * hrmDCBlockY[ch];
                hrmDCBlockX[ch] = outSample;
                hrmDCBlockY[ch] = dcOut;

                osBlock.setSample(ch, n, dcOut);
            }

            hrmPhase += static_cast<float>(twoPi * actualHz / osRate);
            if (hrmPhase >= twoPi) hrmPhase -= twoPi;

            hrmLfoPhase += lfoPhaseInc;
            if (hrmLfoPhase >= twoPi) hrmLfoPhase -= twoPi;
        }

        rmOversampling.processSamplesDown(inputBlock);
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
    param_layout.add(std::make_unique<AudioParameterChoice>("effect", "Effect Type Choice", StringArray("Chorus", "Multi-Voice Chorus", "Ring Modulator", "Harmonic Ring Mod"), 0));

    // return layout
    return param_layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CSC475pitch_effectanalyzerAudioProcessor();
}
