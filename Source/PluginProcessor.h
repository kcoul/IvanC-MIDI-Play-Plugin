/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


//==============================================================================
/**
*/
class SimpleMidiplayerAudioProcessor  : public juce::AudioProcessor, 
                                        private juce::Timer
{
public:
    //==============================================================================
    SimpleMidiplayerAudioProcessor();
    ~SimpleMidiplayerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    /** This function can be called to load a MIDI file so that it can be played. */
    void loadMIDIFile(juce::File fileMIDI);
    
    /** Returns the number of tracks in the MIDI file. */
    int getNumTracks();

    /** Sets the current track from the MIDI file that needs to be played. */
    void setCurrentTrack(int value);

    /** Returns the MIDI file track currently played. */
    int getCurrentTrack();

    /** Callbacks for Play / Pause & Stop button*/
    void pressPlayPauseButton();
    void pressStopButton();
    void pressPanicButton();
    void pressClearLogButton();
    void pressAllTracksCheckBox(bool stateNow);
    void pressOwnTransportCheckBox(bool stateNow);
    void pressLoopCheckBox(bool stateNow);
    void pressSpacerCheckBox(bool stateNow);
    void pressRadioButtonings(int WhichPressed);
    void timerCallback() override;

    bool getUseEntireTracks();
    bool getUseOwnTransport();
    bool getDoLoop();
    bool getDoSpacer();

    juce::String insertMessageLog(juce::String whatToAppend);
    juce::String insertLyricLog(juce::String syllable);

    juce::ScopedPointer<juce::Component> getThisWindowThingy();

    juce::String getFillYourInfoHere();

    juce::ScopedPointer<juce::TextEditor> daInfoTextBox;
    juce::ScopedPointer<juce::Label> daTimerLabel;
    juce::OwnedArray<juce::TextButton> radioButtonings;

private:
    //==============================================================================
    /** Sends Note Off / Controller Off / Sound Off on all the MIDI channels */
    void sendAllNotesOff(juce::MidiBuffer& midiMessages);

    //==============================================================================
    juce::CriticalSection processLock;

    //==============================================================================
    juce::ScopedPointer<juce::Component> thisWindowThingy;

    //==============================================================================
    juce::MidiFile theMIDIFile;
    juce::String lastFilePath;
    bool isPlayingSomething;
    bool trackHasChanged = false;
    bool useEntireTracks = true;
    bool useOwnTransportInstead = false;
    bool myOwnIsPlaying = false;
    bool doLoop = false;
    bool doSpacer = true;
    juce::AudioPlayHead::CurrentPositionInfo thePositionInfo;
    juce::AudioTransportSource ownTransportSource;
    const juce::MidiMessageSequence* entireSequences[16];
    std::atomic<int> currentTrack;
    std::atomic<int> numTracks;
    std::atomic<int> totalNumEvents;
    double ownStartTime;
    double ownOffsetTime;
    double traverseEndTime;
    double ownElapsedTime;
    double lastSampleStartTime;
    double loop;
    double nextStartTime = -1.0;
    int transpose = 0;
    bool haveBeenSpaced = false;
    bool ownIsPlaying = false;
    bool ownStopPlaying = true;
    const unsigned char initialSysExGM [16] =     {0xf0,0x7e,0x7f,0x09,0x01,0xf7};
    const unsigned char initialSysExGMOff [16] =  {0xf0,0x7e,0x7f,0x09,0x02,0xf7};
    const unsigned char initialSysExG2 [16] =     {0xf0,0x7e,0x7f,0x09,0x03,0xf7};
    const unsigned char initialSysExXG [16] =     {0xf0,0x43,0x10,0x4c,0x00,0x00, 0x7e, 0x00, 0xf7 };
    const unsigned char initialSysExGS [16] =     {0xf0,0x41,0x10,0x42,0x12,0x40, 0x00, 0x7f, 0x00, 0x41, 0xf7 };
    const unsigned char initialSysEx88 [16] =     {0xf0,0x41,0x10,0x42,0x12,0x40, 0x00, 0x7f, 0x00, 0x01, 0xf7 };

    juce::MidiMessage initMessageGM = juce::MidiMessage::createSysExMessage(initialSysExGM, 16);
    juce::MidiMessage initMessageGMOff = juce::MidiMessage::createSysExMessage(initialSysExGMOff, 16);
    juce::MidiMessage initMessageG2 = juce::MidiMessage::createSysExMessage(initialSysExG2, 16);
    juce::MidiMessage initMessageXG = juce::MidiMessage::createSysExMessage(initialSysExXG, 16);
    juce::MidiMessage initMessageGS = juce::MidiMessage::createSysExMessage(initialSysExGS, 16);
    juce::MidiMessage initMessage88 = juce::MidiMessage::createSysExMessage(initialSysEx88, 16);
    
    bool tellPlayNow = false;
    bool tellStopNow = false;
    bool tellRecordNow = false;
    bool tellRewindNow = false;
    bool tellWorkaroundFirst = false;
    bool tellLoopPull = false;
    bool tellPanic = false;
    int tellInfoModes = 0;
    int tempFillOutSetMode = 0;

    juce::String fillYourInfoHere = "Perkedel IvanC MIDI Player";
    juce::String fillYourMessageHere = "MESSAGE LOGS\n";
    juce::String fillYourLyricHere = "La la la la la\n";
    juce::String fillYourTimerHere = "0:00 / 0:00";
    juce::Array<juce::String> radioButtoningsSay = {"Info","Messages","Lyrics"};
    juce::Array<juce::String> radioButtoningsTooltips = { "Show Status", "Show MIDI messages log", "Show MIDI lyrics or text,\nbut unfortunately JUCE & many other MIDI Karaoke readers does not recognize Yamaha XF lyrics as Text, warm and bad!" };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMidiplayerAudioProcessor)
};
