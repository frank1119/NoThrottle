/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#include <Windows.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NoThrottleAudioProcessorEditor::NoThrottleAudioProcessorEditor (NoThrottleAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (200, 150);
}

NoThrottleAudioProcessorEditor::~NoThrottleAudioProcessorEditor()
{
}

//==============================================================================
void NoThrottleAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Disable Power Throttling: " + std::string("|/-\\").substr(audioProcessor.prcAdjustCount,1), getLocalBounds(), juce::Justification::centred, 1);
}

void NoThrottleAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
