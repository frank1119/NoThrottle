/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <Windows.h>
#include <JuceHeader.h>

#define Juce_Listener 1

//==============================================================================
/**
*/
class NoThrottleAudioProcessor : public juce::AudioProcessor
							#if JucePlugin_Enable_ARA
							, public juce::AudioProcessorARAExtension
							#endif
							, public juce::Timer
							#if Juce_Listener
							, public juce::AudioProcessorParameter::Listener
							#endif

{
public:
	//==============================================================================
	NoThrottleAudioProcessor();
	~NoThrottleAudioProcessor() override;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

	#if Juce_Listener
	void virtual parameterValueChanged(int, float) override;
	void virtual parameterGestureChanged(int, bool) override;
	#endif
	virtual juce::AudioProcessorParameter* getBypassParameter() const override;

	//juce::AudioProcessorParameter *reservedBypassParameter;
	
	juce::AudioProcessorParameter *gtrl;

	juce::AudioProcessorValueTreeState apvts;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;


	int prcAdjustCount = 0;
	int guiAdjustCount = 0;
	DWORD guiThreadId = 0;


private:
	double samplesPast = 0;
	void timerCallback() override;

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoThrottleAudioProcessor)
};
