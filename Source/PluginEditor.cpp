/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

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

    input_lable.setText("Input Spectrum and chord", juce::dontSendNotification);
    input_lable.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(input_lable);

    output_lable.setText("Output Spectrum and chord", juce::dontSendNotification);
    output_lable.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(output_lable);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(inputSpectrum);
    addAndMakeVisible(inputSpectrogram);
    addAndMakeVisible(outputSpectrum);
    setSize(600, 400);
    startTimerHz(30);

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

    auto top = area.removeFromTop(area.getHeight() / 2);
    auto bottom = area;

    auto leftTop = top.removeFromLeft(top.getWidth() / 2).reduced(10);
    auto rightTop = top.reduced(10);

    inputSpectrum.setBounds(leftTop);
    outputSpectrum.setBounds(rightTop);

    inputSpectrogram.setBounds(bottom.reduced(10));

}

void CSC475pitch_effectanalyzerAudioProcessorEditor::timerCallback()
{
  std::array<float, CSC475pitch_effectanalyzerAudioProcessor::fftSize / 2> mags;
  std::array<float, CSC475pitch_effectanalyzerAudioProcessor::fftSize / 2> outputMags;

  if (audioProcessor.getLatestMagnitudes(mags)){
    inputSpectrum.setMagnitudes(mags);
    inputSpectrogram.pushMagnitudes(mags);

  }
  if (audioProcessor.getLatestOutputMagnitudes(outputMags)){
      outputSpectrum.setMagnitudes(outputMags);
  }
  // auto lv = audioProcessor.inputRms.load(std::memory_order_relaxed);
  // DBG("RMS: " << lv);
  // inputSpectrum.setLevel(lv);

    // Flexbox for over all layout
    juce::FlexBox overall;
    // grid for knob layout
    juce::FlexBox knob_grid;
    // flex for input spectrum section
    juce::FlexBox input_spectrum_box;
    // flex for output spectrum container
    juce::FlexBox output_spectrum_box;

    auto bound = getLocalBounds();
    using fi = juce::FlexItem;

    //create compressor param knob grid
    knob_grid.flexDirection = juce::FlexBox::Direction::row;
    knob_grid.flexWrap = juce::FlexBox::Wrap::wrap;
    knob_grid.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    knob_grid.alignContent = juce::FlexBox::AlignContent::spaceAround;
    //add items
    float grid_item_h = bound.getHeight() / 3.75;
    float grid_item_w = (bound.getWidth() * 0.6) / 3.75;

    knob_grid.items = { juce::FlexItem(rate_knob).withWidth(grid_item_w).withHeight(grid_item_h).withMargin(juce::FlexItem::Margin(0.0, 20.0, 0.0, 20.0)),
                        juce::FlexItem(depth_knob).withWidth(grid_item_w).withHeight(grid_item_h).withMargin(juce::FlexItem::Margin(0.0, 20.0, 0.0, 20.0)),
                        juce::FlexItem(feedback_knob).withWidth(grid_item_w).withHeight(grid_item_h).withMargin(juce::FlexItem::Margin(0.0, 20.0, 0.0, 20.0)),
                        juce::FlexItem(effect).withWidth(grid_item_w).withHeight(grid_item_h).withMargin(juce::FlexItem::Margin(0.0, 20.0, 0.0, 20.0))
    };

    //input spectrum strip layout
    input_spectrum_box.flexDirection = juce::FlexBox::Direction::column;
    input_spectrum_box.flexWrap = juce::FlexBox::Wrap::wrap;
    input_spectrum_box.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    input_spectrum_box.alignContent = juce::FlexBox::AlignContent::spaceAround;

    input_spectrum_box.items = { juce::FlexItem(input_lable).withWidth(bound.getWidth() * 0.175).withHeight(bound.getHeight() * 0.15)

    };

    //output spectrum layout
    output_spectrum_box.flexDirection = juce::FlexBox::Direction::row;
    output_spectrum_box.flexWrap = juce::FlexBox::Wrap::wrap;
    output_spectrum_box.justifyContent = juce::FlexBox::JustifyContent::center;
    output_spectrum_box.alignContent = juce::FlexBox::AlignContent::center;



    output_spectrum_box.items = { juce::FlexItem(output_lable).withWidth(bound.getWidth() * 0.150).withHeight(bound.getHeight() * 0.35) };

    //make overall layout
    overall.flexWrap = juce::FlexBox::Wrap::noWrap;
    overall.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    overall.alignContent = juce::FlexBox::AlignContent::spaceAround;

    overall.items = { juce::FlexItem(input_spectrum_box).withFlex(0.175f).withMargin(juce::FlexItem::Margin(0.0, 0.0, 0.0, 50.0)),
                      juce::FlexItem(knob_grid).withFlex(0.65f).withMargin(juce::FlexItem::Margin(0.0, 15.0, 0.0, 15.0)),
                      juce::FlexItem(output_spectrum_box).withFlex(0.150f).withMargin(juce::FlexItem::Margin(125.0, 50.0, 125.0, 25.0))
    };

    overall.performLayout(getLocalBounds());
}
