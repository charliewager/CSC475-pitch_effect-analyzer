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
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(inputSpectrum);
    addAndMakeVisible(outputSpectrum);
    setSize(600, 400);
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
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::darkgrey);

    g.setColour(juce::Colours::white);
    g.setFont(16.0f);

    auto area = getLocalBounds().reduced(20);
    auto left = area.removeFromLeft(area.getWidth() / 2);
    auto right = area;

    g.drawText("Input Spectrum", left.removeFromTop(5), juce::Justification::centred);
    g.drawText("Output Spectrum", right.removeFromTop(5), juce::Justification::centred);
}

void CSC475pitch_effectanalyzerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds().reduced(20);

    auto left = area.removeFromLeft(area.getWidth() / 2).reduced(10);
    auto right = area.reduced(10);

    inputSpectrum.setBounds(left);
    outputSpectrum.setBounds(right);
    
    inputChordLabel.setBounds(10, getHeight() - 60, 200, 30);
    outputChordLabel.setBounds(10, getHeight() - 30, 200, 30);

}

void CSC475pitch_effectanalyzerAudioProcessorEditor::timerCallback()
{
  std::array<float, CSC475pitch_effectanalyzerAudioProcessor::fftSize / 2> mags;
  std::array<float, CSC475pitch_effectanalyzerAudioProcessor::fftSize / 2> outputMags;

  if (audioProcessor.getLatestMagnitudes(mags)){
    inputSpectrum.setMagnitudes(mags);
  }
  if (audioProcessor.getLatestOutputMagnitudes(outputMags)){
      outputSpectrum.setMagnitudes(outputMags);
  }
  // auto lv = audioProcessor.inputRms.load(std::memory_order_relaxed);
  // DBG("RMS: " << lv);
  // inputSpectrum.setLevel(lv);
    ChordResult inputResult;
    if (audioProcessor.inputChordRecognizer->popResult(inputResult))
        inputChordLabel.setText("Input: " + inputResult.chordName,
                                 juce::dontSendNotification);

    ChordResult outputResult;
    if (audioProcessor.outputChordRecognizer->popResult(outputResult))
        outputChordLabel.setText("Output: " + outputResult.chordName,
                                  juce::dontSendNotification);
}
