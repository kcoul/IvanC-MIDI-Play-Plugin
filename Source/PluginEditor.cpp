#include "PluginProcessor.h"
#include "PluginEditor.h"

SimpleMidiplayerAudioProcessorEditor::SimpleMidiplayerAudioProcessorEditor(SimpleMidiplayerAudioProcessor& p) 
    : juce::AudioProcessorEditor(&p),
    processor(p),
    handedOverFacemask(processor.getThisWindowThingy()),
    infoLabel(*p.daInfoTextBox),
    timerLabel(*p.daTimerLabel),
    radioButtonsPls(p.radioButtonings)
{
    buildDaUI();
}

SimpleMidiplayerAudioProcessorEditor::SimpleMidiplayerAudioProcessorEditor (SimpleMidiplayerAudioProcessor& p, juce::ScopedPointer<juce::Component> componento)
    : juce::AudioProcessorEditor (&p), 
    processor (p),
    infoLabel(*p.daInfoTextBox),
    timerLabel(*p.daTimerLabel),
    radioButtonsPls(p.radioButtonings)
{
    handedOverFacemask = componento;
    buildDaUI();
}

void SimpleMidiplayerAudioProcessorEditor::putFaceMaskInstead() {
}

void SimpleMidiplayerAudioProcessorEditor::buildDaUI() {
    addAndMakeVisible(buttonLoadMIDIFile = new juce::TextButton("Load a MIDI file"));
    buttonLoadMIDIFile->addListener(this);
    addAndMakeVisible(buttonPlayNow = new juce::TextButton("Play / Pause"));
    addAndMakeVisible(buttonStopNow = new juce::TextButton("Stop"));
    addAndMakeVisible(buttonPanicNow = new juce::TextButton("PANIC"));
    addAndMakeVisible(buttonClearLogNow = new juce::TextButton("Clear Logs"));
    addAndMakeVisible(checkBoxAllTracks = new juce::ToggleButton("Entire Tracks"));
    addAndMakeVisible(checkBoxOwnTransport = new juce::ToggleButton("Override Host's Play / pause"));
    addAndMakeVisible(checkBoxLoop = new juce::ToggleButton("Loop"));
    addAndMakeVisible(checkBoxSpacer = new juce::ToggleButton("3 second space at end"));
    addAndMakeVisible(infoLabel);
    addAndMakeVisible(timerLabel);

    for (auto* b : radioButtonsPls)
    {
        addAndMakeVisible(b);
    }

    buttonPlayNow->addListener(this);
    buttonStopNow->addListener(this);
    buttonPanicNow->addListener(this);
    buttonClearLogNow->addListener(this);
    
    buttonPlayNow->setTooltip("Play the song (for Override Playhead) & Signal Pull the loop start");
    buttonStopNow->setTooltip("Stop the song (for Override Playhead) & Cancel signaling pull the loop start");
    buttonPanicNow->setTooltip("Send all notes OFF on all channels.\nif you have hung notes & events, press this button to shock the \"heart\" & stop fibrilation.\n Or Valsava Manuver when playing");
    buttonClearLogNow->setTooltip("Clear all logs & whatever in the info text box");

    checkBoxAllTracks->setButtonText("Entire Tracks");
    checkBoxAllTracks->setTooltip("Toggle whether should all tracks plays");
    checkBoxAllTracks->setToggleState(processor.getUseEntireTracks(), juce::dontSendNotification);
    checkBoxAllTracks->onClick = [this] {
        processor.pressAllTracksCheckBox(checkBoxAllTracks->getToggleState());
    };
    checkBoxOwnTransport->setButtonText("Override Host's Play/Stop head");
    checkBoxOwnTransport->setTooltip("Toggle whether should to use own playhead instead of host's play head");
    checkBoxOwnTransport->setToggleState(processor.getUseOwnTransport(), juce::dontSendNotification);
    checkBoxOwnTransport->onClick = [this] {
        processor.pressOwnTransportCheckBox(checkBoxOwnTransport->getToggleState());
    };
    checkBoxLoop->setButtonText("Loop");
    checkBoxLoop->setTooltip("Toggle whether should the play loops");
    checkBoxLoop->setToggleState(processor.getDoLoop(), juce::dontSendNotification);
    checkBoxLoop->onClick = [this] {
        processor.pressLoopCheckBox(checkBoxLoop->getToggleState());
    };
    checkBoxSpacer->setButtonText("3 second space at end");
    checkBoxSpacer->setTooltip("Toggle whether should to add 3 second space at the end\nWithout this ON, some MIDI files will end with immediate silence (perfectly cut meme)");
    checkBoxSpacer->setToggleState(processor.getDoSpacer(), juce::dontSendNotification);
    checkBoxSpacer->onClick = [this] {
        processor.pressSpacerCheckBox(checkBoxSpacer->getToggleState());
    };

    addAndMakeVisible(comboTrack = new juce::ComboBox());
    comboTrack->addListener(this);
    updateTrackComboBox();
    
    setResizable(true, true);
    setResizeLimits(400, 300, 1280, 720);
    setSize(800, 600);

    juce::RuntimePermissions::request(juce::RuntimePermissions::readExternalStorage,
        [](bool granted)
        {
            if (!granted)
                juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Permissions warning")
                    .withMessage("External storage access permission not granted, some files"
                        " may be inaccessible.")
                    .withButton("OK"),
                    nullptr);
        });
}

SimpleMidiplayerAudioProcessorEditor::~SimpleMidiplayerAudioProcessorEditor()
{
    buttonLoadMIDIFile->removeListener(this);
    buttonPlayNow->removeListener(this);
    buttonStopNow->removeListener(this);
    buttonPanicNow->removeListener(this);
    buttonClearLogNow->removeListener(this);
    checkBoxAllTracks->onClick = NULL;
    checkBoxOwnTransport->onClick = NULL;
    checkBoxLoop->onClick = NULL;
    checkBoxSpacer->onClick = NULL;
    comboTrack->removeListener(this);
}

void SimpleMidiplayerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void SimpleMidiplayerAudioProcessorEditor::resized()
{
    juce::Rectangle<int> rect = getLocalBounds().reduced(4); // legacy

    int daHeight = 22;
    auto columns = rect.removeFromTop(daHeight * 6);
    auto col = columns.removeFromLeft(200);

    buttonLoadMIDIFile->setBounds(col.removeFromTop(daHeight));
    buttonPlayNow->setBounds(col.removeFromTop(daHeight));
    buttonStopNow->setBounds(col.removeFromTop(daHeight));
    buttonPanicNow->setBounds(col.removeFromTop(daHeight));
    buttonClearLogNow->setBounds(col.removeFromTop(daHeight));
    
    columns.removeFromLeft(20);
    col = columns.removeFromLeft(200);
    
    checkBoxAllTracks->setBounds(col.removeFromTop(daHeight));
    checkBoxOwnTransport->setBounds(col.removeFromTop(daHeight));
    checkBoxLoop->setBounds(col.removeFromTop(daHeight));
    checkBoxSpacer->setBounds(col.removeFromTop(daHeight));

    rect.removeFromBottom(7);
    comboTrack->setBounds(rect.removeFromTop(daHeight));
    timerLabel.setBounds(rect.removeFromTop(50));
    columns = rect.removeFromTop(daHeight); 
    col = columns.removeFromLeft(500);

    for (auto* b : radioButtonsPls) {
        b->setBounds(col.removeFromLeft(100));
    }
    infoLabel.setBounds(rect.removeFromTop(400));
}

void SimpleMidiplayerAudioProcessorEditor::buttonClicked(juce::Button * button)
{
    if (button == buttonLoadMIDIFile)
    {
        juce::FileChooser theFileChooser("Find a MIDI file", juce::File(), "*.mid*");
        
        if (theFileChooser.browseForFileToOpen()) 
        {
            processor.loadMIDIFile(theFileChooser.getResult());
            updateTrackComboBox();
        }
    }
    else if (button == buttonPlayNow) {
        processor.pressPlayPauseButton();
    }
    else if (button == buttonStopNow) {
        processor.pressStopButton();
    }
    else if (button == buttonPanicNow) {
        processor.pressPanicButton();
    }
    else if (button == buttonClearLogNow) {
        processor.pressClearLogButton();
    }
}

void SimpleMidiplayerAudioProcessorEditor::comboBoxChanged(juce::ComboBox* combo)
{
    if (combo == comboTrack)
    {
        processor.setCurrentTrack(combo->getSelectedId() - 1);
    }
}

void SimpleMidiplayerAudioProcessorEditor::updateTrackComboBox()
{
    comboTrack->clear();

    for (auto i = 0; i < processor.getNumTracks(); i++)
        comboTrack->addItem("Track number " + juce::String(i + 1), i + 1);

    comboTrack->setSelectedId(processor.getCurrentTrack() + 1, juce::dontSendNotification);
}

void SimpleMidiplayerAudioProcessorEditor::setInfoLabelText(juce::String daNewText) {
    infoLabel.setText(daNewText,juce::dontSendNotification);
}

juce::String SimpleMidiplayerAudioProcessorEditor::getInfoLabelText() {
    return infoLabel.getText();
}
