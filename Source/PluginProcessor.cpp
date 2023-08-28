/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define UNREFERENCED_PARAMETER(P) (P)

#pragma region Get GUI ThreadId
struct handle_data {
	unsigned long process_id;
	HWND window_handle;
};

BOOL is_main_window(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle))
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}

HWND find_main_window(unsigned long process_id)
{
	handle_data data = {};
	data.process_id = process_id;
	data.window_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.window_handle;
}
#pragma endregion

//==============================================================================

using Parameter = juce::AudioProcessorValueTreeState::Parameter;

NoThrottleAudioProcessor::NoThrottleAudioProcessor() :
	apvts(*this, nullptr, "PARAMETERS", {
		std::make_unique<juce::AudioParameterChoice>("gtrl", "GUI Throttling", juce::StringArray{"On", "Off", "Default", "Skip"}, 3)
		})
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
#endif
{
	gtrl = apvts.getParameter("gtrl");

	gtrl->addListener(this);

	HWND hwnd = find_main_window(GetCurrentProcessId());
	guiThreadId = GetWindowThreadProcessId(hwnd, NULL);
	startTimerHz(4);

}

NoThrottleAudioProcessor::~NoThrottleAudioProcessor()
{
	stopTimer();
}

//==============================================================================
const juce::String NoThrottleAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool NoThrottleAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool NoThrottleAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool NoThrottleAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double NoThrottleAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int NoThrottleAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int NoThrottleAudioProcessor::getCurrentProgram()
{
	return 0;
}

void NoThrottleAudioProcessor::setCurrentProgram(int index)
{
	UNREFERENCED_PARAMETER(index);
}

const juce::String NoThrottleAudioProcessor::getProgramName(int index)
{
	UNREFERENCED_PARAMETER(index);
	return {};
}

void NoThrottleAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
	UNREFERENCED_PARAMETER(index);
	UNREFERENCED_PARAMETER(newName);
}

//==============================================================================
void NoThrottleAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	UNREFERENCED_PARAMETER(sampleRate);
	UNREFERENCED_PARAMETER(samplesPerBlock);
	// Use this method as the place to do any pre-playback
	// initialisation that you need..
}

void NoThrottleAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
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

juce::AudioProcessorParameter* NoThrottleAudioProcessor::getBypassParameter() const
{
	// The only way to get rid of the automatic 'Bypass' parameter
	return gtrl;
}

void NoThrottleAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(midiMessages);
	juce::ScopedNoDenormals noDenormals;

	samplesPast += buffer.getNumSamples();
	double srate = getSampleRate();

	if (samplesPast > srate)
	{
		samplesPast -= srate;
		THREAD_POWER_THROTTLING_STATE throttling_state;
		memset(&throttling_state, 0, sizeof(throttling_state));
		throttling_state.Version = THREAD_POWER_THROTTLING_CURRENT_VERSION;
		throttling_state.ControlMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;
		throttling_state.StateMask = 0;
		SetThreadInformation(GetCurrentThread(), ThreadPowerThrottling, &throttling_state, sizeof(throttling_state));
		prcAdjustCount = (prcAdjustCount + 1) % 4;
	}

	// In case we have more outputs than inputs, this code clears any output
	// channels that didn't contain input data, (because these aren't
	// guaranteed to be empty - they may contain garbage).
	// This is here to avoid people getting screaming feedback
	// when they first compile a plugin, but obviously you don't need to keep
	// this code if your algorithm always overwrites all the output channels.
	//for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
	//	buffer.clear(i, 0, buffer.getNumSamples());

	// This is the place where you'd normally do the guts of your plugin's
	// audio processing...
	// Make sure to reset the state if your inner loop is processing
	// the samples and the outer loop is handling the channels.
	// Alternatively, you can process the samples with the channels
	// interleaved by keeping the same state.
	//for (int channel = 0; channel < totalNumInputChannels; ++channel)
	//{
	//	auto* channelData = buffer.getWritePointer(channel);
	//
	//	// ..do something to the data...
	//}
}

//==============================================================================
bool NoThrottleAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NoThrottleAudioProcessor::createEditor()
{
	return new NoThrottleAudioProcessorEditor(*this);
}

//==============================================================================
void NoThrottleAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	UNREFERENCED_PARAMETER(destData);

	MemoryOutputStream(destData, true).writeFloat(gtrl->getValue());

	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
}

void NoThrottleAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	UNREFERENCED_PARAMETER(data);
	UNREFERENCED_PARAMETER(sizeInBytes);

	gtrl->setValueNotifyingHost(MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readFloat());
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new NoThrottleAudioProcessor();
}

void NoThrottleAudioProcessor::timerCallback()
{

	if ((guiAdjustCount % 4) == 0)
	{
		juce::AudioParameterChoice* d = (juce::AudioParameterChoice*)gtrl;
		int choice = d->getIndex();

		if (choice < 3) // Skip means -> stay away from it
		{
			THREAD_POWER_THROTTLING_STATE throttling_state;
			memset(&throttling_state, 0, sizeof(throttling_state));

			switch (choice)
			{
			case 0: // On
				throttling_state.Version = THREAD_POWER_THROTTLING_CURRENT_VERSION;
				throttling_state.ControlMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;
				throttling_state.StateMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;;
				break;
			case 1: // Off
				throttling_state.Version = THREAD_POWER_THROTTLING_CURRENT_VERSION;
				throttling_state.ControlMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;
				throttling_state.StateMask = 0;
				break;
			default: // Default
				throttling_state.Version = THREAD_POWER_THROTTLING_CURRENT_VERSION;
				throttling_state.ControlMask = 0;
				throttling_state.StateMask = 0;
				break;
			}

			HANDLE threadHandle = OpenThread(THREAD_ALL_ACCESS, FALSE, guiThreadId);
			if (threadHandle != NULL)
			{
				SetThreadInformation(threadHandle, ThreadPowerThrottling, &throttling_state, sizeof(throttling_state));
				CloseHandle(threadHandle);
			}
		}
	}

	guiAdjustCount = (guiAdjustCount + 1) % 4;
	juce::AudioProcessorEditor* ae = getActiveEditor();
	if (ae != nullptr)
		ae->repaint();
}

void NoThrottleAudioProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
	UNREFERENCED_PARAMETER(newValue);
	if (parameterIndex == gtrl->getParameterIndex())
	{
		juce::AudioParameterChoice* d = (juce::AudioParameterChoice*)gtrl;

		int choice = d->getIndex();

		if (choice == 3 && gtrl_val != 3) // skip. Reset once to default and then nothing
		{
			THREAD_POWER_THROTTLING_STATE throttling_state;
			memset(&throttling_state, 0, sizeof(throttling_state));
			throttling_state.Version = THREAD_POWER_THROTTLING_CURRENT_VERSION;
			throttling_state.ControlMask = 0;
			throttling_state.StateMask = 0;

			HANDLE threadHandle = OpenThread(THREAD_ALL_ACCESS, FALSE, guiThreadId);
			if (threadHandle != NULL)
			{
				SetThreadInformation(threadHandle, ThreadPowerThrottling, &throttling_state, sizeof(throttling_state));
				CloseHandle(threadHandle);
			}
		}
		gtrl_val = choice;
	}
}

void NoThrottleAudioProcessor::parameterGestureChanged(int, bool)
{
}
