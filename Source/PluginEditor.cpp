/*
  ==============================================================================



  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
CSC475pitch_effectanalyzerAudioProcessorEditor::CSC475pitch_effectanalyzerAudioProcessorEditor (CSC475pitch_effectanalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // configure knobs
    //rate_knob.setLookAndFeel(&m_3dKnobLook);
    rate_knob.setTextValueSuffix("Hz");
    rate_knob.setRange(0.1, 2.0, 0.001);
    addAndMakeVisible(rate_knob);

    r_label.setText("Rate", juce::dontSendNotification);
    r_label.setJustificationType(juce::Justification::centred);
    r_label.attachToComponent(&rate_knob, false);
    addAndMakeVisible(r_label);

    // add attachment
    rateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts,
        "rate",
        rate_knob
    );

    //depth_knob.setLookAndFeel(&m_3dKnobLook);
    //depth_knob.setTextValueSuffix("Hz");
    depth_knob.setRange(0, 1, 0.001);
    addAndMakeVisible(depth_knob);

    d_label.setText("Depth", juce::dontSendNotification);
    d_label.setJustificationType(juce::Justification::centred);
    d_label.attachToComponent(&depth_knob, false);
    addAndMakeVisible(d_label);

    // add attachment
    depthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts,
        "depth",
        depth_knob
    );

    //feedback_knob.setLookAndFeel(&m_3dKnobLook);
    feedback_knob.setTextValueSuffix("Hz");
    feedback_knob.setRange(-0.25, 0.25, 0.001);
    addAndMakeVisible(feedback_knob);

    f_label.setText("Feedback", juce::dontSendNotification);
    f_label.setJustificationType(juce::Justification::centred);
    f_label.attachToComponent(&feedback_knob, false);
    addAndMakeVisible(f_label);

    // add attachment
    feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts,
        "feedback",
        feedback_knob
    );

    effect.addItem("Chorus", 1);
    effect.addItem("Multi-Voice Chorus", 2);
    effect.addItem("Ring Modulator", 3);
    effect.addItem("Harmonic Ring Mod", 4);
    effect.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(effect);

    // add attachment
    effectAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts,
        "effect",
        effect
    );

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(inputSpectrum);
    addAndMakeVisible(inputSpectrogram);
    addAndMakeVisible(outputSpectrum);
    setSize(900, 660);
    startTimerHz(30);
    inputChordLabel.setText("Input: N/C", juce::dontSendNotification);
    inputChordLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    inputChordLabel.setFont(juce::FontOptions(20.f, juce::Font::bold));
    addAndMakeVisible(inputChordLabel);

    outputChordLabel.setText("Output: N/C", juce::dontSendNotification);
    outputChordLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
    outputChordLabel.setFont(juce::FontOptions(20.f, juce::Font::bold));
    addAndMakeVisible(outputChordLabel);

}

CSC475pitch_effectanalyzerAudioProcessorEditor::~CSC475pitch_effectanalyzerAudioProcessorEditor()
{
}

//==============================================================================
void CSC475pitch_effectanalyzerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void CSC475pitch_effectanalyzerAudioProcessorEditor::resized()
{
    const int pad = 10;
    const int labelHeight = 20;

    auto area = getLocalBounds().reduced(pad);

    // Bottom 40%: scrolling spectrogram + chord label row
    auto spectrogramArea = area.removeFromBottom(static_cast<int>(area.getHeight() * 0.4f));

    // Chord labels below spectrogram, split left (input) / right (output)
    auto chordLabelArea = spectrogramArea.removeFromBottom(28);
    inputChordLabel .setBounds(chordLabelArea.removeFromLeft(chordLabelArea.getWidth() / 2));
    outputChordLabel.setBounds(chordLabelArea);

    inputSpectrogram.setBounds(spectrogramArea.reduced(pad));

    // Top 60%: input spectrum | knob grid | output spectrum
    const int totalWidth = area.getWidth();
    auto inputSpectrumArea  = area.removeFromLeft (static_cast<int>(totalWidth * 0.25f));
    auto outputSpectrumArea = area.removeFromRight(static_cast<int>(totalWidth * 0.25f));
    auto knobArea           = area; // middle 50%

    inputSpectrum .setBounds(inputSpectrumArea .reduced(pad));
    outputSpectrum.setBounds(outputSpectrumArea.reduced(pad));

    // Knob area: headroom for attached labels, then 3 knobs, then dropdown
    knobArea.removeFromTop(labelHeight);
    knobArea = knobArea.reduced(pad);

    const int dropdownH = 35;
    const int dropdownW = 220;
    const int knobH     = knobArea.getHeight() - dropdownH - pad;
    const int knobW     = knobArea.getWidth() / 3;

    auto knobRow = knobArea.removeFromTop(knobH);
    rate_knob    .setBounds(knobRow.removeFromLeft(knobW).reduced(4));
    depth_knob   .setBounds(knobRow.removeFromLeft(knobW).reduced(4));
    feedback_knob.setBounds(knobRow.reduced(4));

    knobArea.removeFromTop(pad);
    effect.setBounds(knobArea.getCentreX() - dropdownW / 2,
                     knobArea.getY(),
                     dropdownW, dropdownH);

}

void CSC475pitch_effectanalyzerAudioProcessorEditor::timerCallback()
{
    // Forward sample rate to spectrum components whenever it changes
    const double sr = audioProcessor.getSampleRate();
    if (sr > 0.0 && sr != lastSampleRate)
    {
        lastSampleRate = sr;
        inputSpectrum.setSampleRate(sr);
        outputSpectrum.setSampleRate(sr);
    }

    std::array<float, CSC475pitch_effectanalyzerAudioProcessor::fftSize / 2> mags;
    std::array<float, CSC475pitch_effectanalyzerAudioProcessor::fftSize / 2> outputMags;

    if (audioProcessor.getLatestMagnitudes(mags)) {
        inputSpectrum.setMagnitudes(mags);
        inputSpectrogram.pushMagnitudes(mags);
    }
    if (audioProcessor.getLatestOutputMagnitudes(outputMags)) {
        outputSpectrum.setMagnitudes(outputMags);
    }

    ChordResult inputResult;
    if (audioProcessor.inputChordRecognizer->popResult(inputResult))
        inputChordLabel.setText("Input: " + inputResult.chordName,
                                 juce::dontSendNotification);

    ChordResult outputResult;
    if (audioProcessor.outputChordRecognizer->popResult(outputResult))
        outputChordLabel.setText("Output: " + outputResult.chordName,
                                  juce::dontSendNotification);
}
