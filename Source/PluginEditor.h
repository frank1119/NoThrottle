/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

using namespace juce;

template <typename Func, typename... Items>
constexpr void forEach(Func&& func, Items&&... items)
{
    (func(std::forward<Items>(items)), ...);
}

template <typename... Components>
void addAllAndMakeVisible(Component& target, Components&... children)
{
    forEach([&](Component& child) { target.addAndMakeVisible(child); }, children...);
}

class AttachedCombo : public Component
{
public:
    AttachedCombo(RangedAudioParameter& paramIn)
        : 
        combo(paramIn),
        label("", paramIn.name),
        attachment(paramIn, combo)
    {
        addAllAndMakeVisible(*this, combo, label);
        label.attachToComponent(&combo, false);
        label.setJustificationType(Justification::centredLeft); 
        boundsOffsets = juce::Rectangle(0, label.getBounds().getHeight(), 150, 24);
    }

    void resized() override
    {
        combo.setBounds(boundsOffsets);
    }
    /// <summary>
    /// Use this to adjust placement of the combobox itself
    /// </summary>
    juce::Rectangle<int> boundsOffsets;

private:
    struct ComboWithItems : public ComboBox
    {
        explicit ComboWithItems(RangedAudioParameter& param)
        {
            // Adding the list here in the constructor means that the combo
            // is already populated when we construct the attachment below
            addItemList(dynamic_cast<juce::AudioParameterChoice&> (param).choices, 1);
        }
    };

    ComboWithItems combo;
    Label label;
    ComboBoxParameterAttachment attachment;
};

//==============================================================================



//==============================================================================
/**
*/
class NoThrottleAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NoThrottleAudioProcessorEditor (NoThrottleAudioProcessor&);
    ~NoThrottleAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NoThrottleAudioProcessor& audioProcessor;

    std::unique_ptr <AttachedCombo> cmbGuiThrottle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoThrottleAudioProcessorEditor)
};
