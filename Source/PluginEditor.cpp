/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
//#include <Windows.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NoThrottleAudioProcessorEditor::NoThrottleAudioProcessorEditor(NoThrottleAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	setSize(300, 160);

	cmbGuiThrottle.reset(new AttachedCombo(*(juce::RangedAudioParameter*)p.gtrl));
	addAllAndMakeVisible(*this, *cmbGuiThrottle);
	juce::Rectangle r = getLocalBounds();
	r.setHeight((int)(((float)r.getHeight()) * 0.66));
	
	cmbGuiThrottle->setBounds(r.withPosition(10,5));
	
}

NoThrottleAudioProcessorEditor::~NoThrottleAudioProcessorEditor()
{
}

//==============================================================================
void NoThrottleAudioProcessorEditor::paint(juce::Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	//g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

	g.fillAll(juce::Colours::slategrey);
	g.setColour(juce::Colours::white);
	g.setFont(15.0f);
	juce::Rectangle r = getLocalBounds();
	r.setHeight((int)(((float)r.getHeight()) * 0.9f));
	g.drawLine((float)r.getX() + 3.0f, (float)r.getHeight() * .8f, (float)r.getWidth() - 4.0f, (float)r.getHeight() * .8f);
	g.drawFittedText("Disable Power Throttling: " + std::string("|/-\\").substr(audioProcessor.prcAdjustCount, 1), r.withLeft(10), juce::Justification::bottomLeft, 1);
}

void NoThrottleAudioProcessorEditor::resized()
{
	// This is generally where you'll want to lay out the positions of any
	// subcomponents in your editor..
}
