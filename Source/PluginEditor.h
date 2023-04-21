#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class SimpleMidiplayerAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Button::Listener, public juce::ComboBox::Listener
{
public:
    //==============================================================================
    SimpleMidiplayerAudioProcessorEditor (SimpleMidiplayerAudioProcessor&);
    SimpleMidiplayerAudioProcessorEditor (SimpleMidiplayerAudioProcessor&,juce::ScopedPointer<juce::Component>);
    ~SimpleMidiplayerAudioProcessorEditor();
    void buildDaUI();
    void putFaceMaskInstead();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    void buttonClicked(juce:: Button* button) override;
    void comboBoxChanged(juce::ComboBox* combo) override;

    void setInfoLabelText(juce::String daNewText);
    juce::String getInfoLabelText();

private:
    //==============================================================================
    void updateTrackComboBox();

    juce::ScopedPointer<juce::TextButton> buttonLoadMIDIFile;
    juce::ScopedPointer<juce::ComboBox> comboTrack;
    juce::ScopedPointer<juce::TextButton>   buttonPlayNow,
                                            buttonStopNow,
                                            buttonPanicNow,
                                            buttonClearLogNow
                                            ;
    juce::ScopedPointer<juce::ToggleButton> checkBoxAllTracks,
                                            checkBoxOwnTransport,
                                            checkBoxLoop,
                                            checkBoxSpacer
                                            ;
    juce::TextEditor&                       infoLabel
                                            ;
    juce::Label&                            timerLabel
                                            ;
    juce::OwnedArray<juce::TextButton>&     radioButtonsPls
                                            ;
    
    //==============================================================================
    SimpleMidiplayerAudioProcessor& processor;
    juce::ScopedPointer<juce::Component> handedOverFacemask;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMidiplayerAudioProcessorEditor)
};
