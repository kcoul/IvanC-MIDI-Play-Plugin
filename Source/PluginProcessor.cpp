#include "PluginProcessor.h"
#include "PluginEditor.h"

SimpleMidiplayerAudioProcessor::SimpleMidiplayerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif
    ownStartTime(juce::Time::getMillisecondCounterHiRes() * 0.001),
    daInfoTextBox(new juce::TextEditor),
    daTimerLabel(new juce::Label("Timer","0:00 / 0:00"))
{
    juce::AudioTransportSource ownTransportSource();

    daInfoTextBox->setReturnKeyStartsNewLine(false);
    daInfoTextBox->setMultiLine(true);
    daInfoTextBox->setReadOnly(true);
    daInfoTextBox->setScrollbarsShown(true);
    daInfoTextBox->setCaretVisible(true);
    daInfoTextBox->setPopupMenuEnabled(false);
    daInfoTextBox->setText(fillYourInfoHere, juce::dontSendNotification);
    daTimerLabel->setText(fillYourTimerHere, juce::dontSendNotification);
    daTimerLabel->setSize(100, 50);
    
    for (int i = 0; i < radioButtoningsSay.size(); i++)
    {
        auto* b = radioButtonings.add(new juce::TextButton(radioButtoningsSay[i], radioButtoningsTooltips[i]));
        b->setRadioGroupId(77);
        b->setClickingTogglesState(true);
        b->setTooltip(radioButtoningsTooltips[i]);
        b->onClick = std::bind([this](int a) {
            tellInfoModes = a;
            pressRadioButtonings(a);
            }, i);

        b->setConnectedEdges(
            ((i != 0) ? juce::Button::ConnectedOnLeft : 0) 
            | 
            ((i != radioButtoningsSay.size()-1) ? juce::Button::ConnectedOnRight : 0))
            ;
    }

    numTracks.store(0);
}

SimpleMidiplayerAudioProcessor::~SimpleMidiplayerAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleMidiplayerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleMidiplayerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleMidiplayerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleMidiplayerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleMidiplayerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleMidiplayerAudioProcessor::getNumPrograms()
{
    return 1;
}

int SimpleMidiplayerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleMidiplayerAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String SimpleMidiplayerAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void SimpleMidiplayerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index);
    juce::ignoreUnused(newName);
}

//==============================================================================
void SimpleMidiplayerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate);
    juce::ignoreUnused(samplesPerBlock);

    nextStartTime = -1.0;
}

void SimpleMidiplayerAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleMidiplayerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleMidiplayerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = 0; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const juce::ScopedTryLock myScopedLock(processLock);

    ownTransportSource.getNextAudioBlock(juce::AudioSourceChannelInfo(buffer));

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
    }

    if (tellPlayNow)
    {
        if (useOwnTransportInstead) {
            ownStopPlaying = false;
            ownIsPlaying = !ownIsPlaying;
            if (ownIsPlaying) {

            }
            else {

            }
        }
        else {

        }
        tellPlayNow = false;
    }

    if (tellStopNow)
    {
        if (useOwnTransportInstead){
            ownIsPlaying = false;
            ownStopPlaying = true;
        }
        else{

        }

        tellStopNow = false;
    }

    if (tellRecordNow)
    {
        tellRecordNow = false;
    }

    if (tellRewindNow) {
        tellRewindNow = false;
    }

    if (tellPanic) {
        sendAllNotesOff(midiMessages);
        tellPanic = false;
    }

    if (tellWorkaroundFirst) {

    }
    else {
        initMessageGM.setTimeStamp(0);
        midiMessages.addEvent(initMessageGM, 0);

        auto daDrumPatch = juce::MidiMessage::programChange(10, 127);
        daDrumPatch.setTimeStamp(0);

        auto daSysEx = juce::MidiMessage::createSysExMessage(initialSysExG2, 16);
        daSysEx.setTimeStamp(0);
        midiMessages.addEvent(daSysEx, 0);

        tellWorkaroundFirst = true;
    }

    if (myScopedLock.isLocked())
    {
        getPlayHead()->getCurrentPosition(thePositionInfo);
        
        if (numTracks.load() > 0)
        {
            if (useOwnTransportInstead) {
                if (ownIsPlaying) {
                    const juce::MidiMessageSequence* theSequence = theMIDIFile.getTrack(currentTrack.load());

                    ownOffsetTime = fmod(ownStartTime, traverseEndTime);
                    ownStartTime = thePositionInfo.timeInSeconds - ownOffsetTime;
                    ownElapsedTime += 0;
                }
                else {
                    ownOffsetTime = thePositionInfo.timeInSeconds;
                }
            } else{
                if (thePositionInfo.isPlaying)
                {
                    const juce::MidiMessageSequence* theSequence = theMIDIFile.getTrack(currentTrack.load());

                    auto startTime = thePositionInfo.timeInSeconds;

                    double loopOffset = fmod(startTime, traverseEndTime);
                    loop = startTime - loopOffset;
                    auto sampleStartTime = startTime - (doLoop? loop: 0.0);

                    if (lastSampleStartTime > sampleStartTime)
                    {
                        sampleStartTime = 0.0;
                        tellLoopPull = false;
                    }

                    lastSampleStartTime = sampleStartTime;
                    auto sampleEndTime = sampleStartTime + buffer.getNumSamples() / getSampleRate();
                    auto sampleLength = 1.0 / getSampleRate();

                    if (std::abs(sampleStartTime - nextStartTime) > sampleLength && nextStartTime > 0.0)
                        sendAllNotesOff(midiMessages);

                    nextStartTime = sampleEndTime;
                    
                    if (isPlayingSomething && sampleStartTime >= traverseEndTime)
                        sendAllNotesOff(midiMessages);
                    else
                    {
                        if (trackHasChanged)
                        {
                            trackHasChanged = false;
                            if (!useEntireTracks)
                                sendAllNotesOff(midiMessages);
                        }
                        int curTranspose = transpose, lastTranspose = transpose;

                        for (int i = 0; i < (useEntireTracks? numTracks.load() : 1); i++) {
                            for (auto j = 0; j < (useEntireTracks? entireSequences[i]->getNumEvents() : theSequence->getNumEvents()); j++) {
                                juce::MidiMessageSequence::MidiEventHolder* event = useEntireTracks? entireSequences[i]->getEventPointer(j) : theSequence->getEventPointer(j);

                                if (event->message.getTimeStamp() >= sampleStartTime && event->message.getTimeStamp() < sampleEndTime)
                                {
                                    auto samplePosition = juce::roundToInt((event->message.getTimeStamp() - sampleStartTime) * getSampleRate());
                                    midiMessages.addEvent(event->message, samplePosition);
                                    insertMessageLog(juce::String(samplePosition) + "\t\t -> " + juce::String(event->message.getDescription()));
                                    if (event->message.isTextMetaEvent())
                                        insertLyricLog(event->message.getTextFromTextMetaEvent());

                                    if (fmod(startTime, traverseEndTime) > fmod(sampleEndTime, traverseEndTime))
                                    {
                                        juce::MidiMessageSequence::MidiEventHolder event2 = useEntireTracks? *entireSequences[i]->getEventPointer(0) : *theSequence->getEventPointer(0);
                                        auto samplePosition = juce::roundToInt((event2.message.getTimeStamp()) * getSampleRate());
                                        midiMessages.addEvent(event2.message, samplePosition);
                                    }

                                    isPlayingSomething = true;
                                }
                            }
                        }
                    }
                    
                }
                else
                {
                    if (isPlayingSomething)
                        sendAllNotesOff(midiMessages);
                }
            }
        }
    }
    else
    {
        if (isPlayingSomething)
            sendAllNotesOff(midiMessages);        
    }

    fillYourInfoHere += ("Play status = " + juce::String(isPlayingSomething ? "PLAYING" : "STOPPED") + "\n");
    fillYourInfoHere += "MIDI file = "+ lastFilePath +"\n";
    fillYourInfoHere += "Using Own Transport = "+ juce::String(useOwnTransportInstead ? "YES" : "NO") + "\n";
    fillYourInfoHere += "Using Entire Track = "+ juce::String(useEntireTracks ? "YES" : "NO") + "\n";
    fillYourInfoHere += "Total Tracks = " + juce::String(numTracks.load()) + "\n";
    fillYourInfoHere += "Chosen Track = "+ juce::String(currentTrack.load()) + "\n";
    fillYourInfoHere += "Total End time = "+ juce::String(traverseEndTime) + "\n";
    fillYourInfoHere += "Radio button selected = " + juce::String(tellInfoModes) + "\n";

    fillYourTimerHere = "Playhead Time \t\t= " + juce::String(thePositionInfo.timeInSeconds) + "\nSample Start Time \t\t= " + juce::String(lastSampleStartTime) + " / " + juce::String(traverseEndTime);

    juce::String yoWhatToShow = "";
    switch (tellInfoModes) {
    case 0:
        yoWhatToShow = fillYourInfoHere;
        break;
    case 1:
        yoWhatToShow = fillYourMessageHere;
        break;
    case 2:
        yoWhatToShow = fillYourLyricHere;
        break;
    default:
        yoWhatToShow = fillYourInfoHere;
        break;
    }

    if(tellInfoModes == 0)
        daInfoTextBox->setText(yoWhatToShow, juce::dontSendNotification);
    daTimerLabel->setText(fillYourTimerHere, juce::dontSendNotification);
}

//==============================================================================
bool SimpleMidiplayerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleMidiplayerAudioProcessor::createEditor()
{
    return new SimpleMidiplayerAudioProcessorEditor(*this, thisWindowThingy);
}

//==============================================================================
void SimpleMidiplayerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
}

void SimpleMidiplayerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data);
    juce::ignoreUnused(sizeInBytes);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleMidiplayerAudioProcessor();
}

//==============================================================================
void SimpleMidiplayerAudioProcessor::loadMIDIFile(juce::File fileMIDI)
{
    const juce::ScopedLock myScopedLock(processLock);

    theMIDIFile.clear();
    
    juce::FileInputStream theStream(fileMIDI);
    theMIDIFile.readFrom(theStream);
    lastFilePath = theStream.getFile().getFileName();

    /** This function call means that the MIDI file is going to be played with the
        original tempo and signature.

        To play it at the host tempo, we might need to do it manually in processBlock
        and retrieve all the time the current tempo to track tempo changes.
    */
    theMIDIFile.convertTimestampTicksToSeconds();
    
    numTracks.store(theMIDIFile.getNumTracks());
    currentTrack.store(0);
    trackHasChanged = false;
    traverseEndTime = theMIDIFile.getLastTimestamp() + (doSpacer? 3.00 : 0.00);
    haveBeenSpaced = doSpacer;
    totalNumEvents = 0;
    fillYourMessageHere = "MIDI file = " + lastFilePath;
    fillYourLyricHere = "\n===========================\n" + lastFilePath + "\n===========================\n\n";
    pressRadioButtonings(tellInfoModes);

    for (int i = 0; i < numTracks.load(); i++)
    {
        entireSequences[i] = theMIDIFile.getTrack(i);
        totalNumEvents += entireSequences[i]->getNumEvents();
    }

    juce::MidiMessage::programChange(10, 127);

    tellWorkaroundFirst = false;
}

void SimpleMidiplayerAudioProcessor::pressPlayPauseButton() {
    if (useOwnTransportInstead) {
        if (Timer::isTimerRunning())
        {

        }
        else {
            printf("Play lah!");
            DBG("Play lah!");
        }
    }
    tellPlayNow = true;
    tellLoopPull = true;
}

void SimpleMidiplayerAudioProcessor::pressStopButton() {
    if (useOwnTransportInstead) {
        stopTimer();
    }
    tellStopNow = true;
    tellLoopPull = false;
}

void SimpleMidiplayerAudioProcessor::pressPanicButton() {
    tellPanic = true;
}

void SimpleMidiplayerAudioProcessor::pressClearLogButton() {
    fillYourInfoHere.clear();
    fillYourLyricHere.clear();
    fillYourMessageHere.clear();

    daInfoTextBox->clear();
}

void SimpleMidiplayerAudioProcessor::pressAllTracksCheckBox(bool stateNow) {
    useEntireTracks = stateNow;
}

void SimpleMidiplayerAudioProcessor::pressOwnTransportCheckBox(bool stateNow) {
    useOwnTransportInstead = stateNow;
}

void SimpleMidiplayerAudioProcessor::pressLoopCheckBox(bool stateNow) {
    doLoop = stateNow;
    tellLoopPull = doLoop;
}

void SimpleMidiplayerAudioProcessor::pressSpacerCheckBox(bool stateNow) {
    doSpacer = stateNow;
    if (haveBeenSpaced)
        traverseEndTime = doSpacer ? traverseEndTime - 3 : traverseEndTime;
        traverseEndTime = doSpacer ? traverseEndTime + 3 : traverseEndTime;
    haveBeenSpaced = doSpacer;
}

void SimpleMidiplayerAudioProcessor::pressRadioButtonings(int WhichPressed)
{
    DBG("Pressed" + WhichPressed);
    switch (WhichPressed) {
    case 0:
        daInfoTextBox->setText(fillYourInfoHere, juce::dontSendNotification);
        break;
    case 1:
        daInfoTextBox->setText(fillYourMessageHere, juce::dontSendNotification);
        break;
    case 2:
        daInfoTextBox->setText(fillYourLyricHere, juce::dontSendNotification);
        break;
    default:
        daInfoTextBox->setText(fillYourInfoHere, juce::dontSendNotification);
        break;
    }
    daInfoTextBox->setCaretPosition(daInfoTextBox->getText().length());
}

bool SimpleMidiplayerAudioProcessor::getUseEntireTracks() {
    return useEntireTracks;
}

bool SimpleMidiplayerAudioProcessor::getUseOwnTransport() {
    return useOwnTransportInstead;
}

bool SimpleMidiplayerAudioProcessor::getDoLoop() {
    return doLoop;
}

bool SimpleMidiplayerAudioProcessor::getDoSpacer() {
    return doSpacer;
}

int SimpleMidiplayerAudioProcessor::getNumTracks()
{
    return numTracks.load();
}

void SimpleMidiplayerAudioProcessor::setCurrentTrack(int value)
{
    jassert(value >= 0 && value < numTracks.load());
    
    if (numTracks.load() == 0)
        return;

    currentTrack.store(value);
    trackHasChanged = true;
}

int SimpleMidiplayerAudioProcessor::getCurrentTrack()
{
    if (numTracks.load() == 0)
        return -1;

    else
        return currentTrack.load();
}

void SimpleMidiplayerAudioProcessor::sendAllNotesOff(juce::MidiBuffer& midiMessages)
{
    for (auto i = 1; i <= 16; i++)
    {
        midiMessages.addEvent(juce::MidiMessage::allNotesOff(i), 0);
        midiMessages.addEvent(juce::MidiMessage::allSoundOff(i), 0);
        midiMessages.addEvent(juce::MidiMessage::allControllersOff(i), 0);
    }

    isPlayingSomething = false;
    tellWorkaroundFirst = false;
    tellLoopPull = false;
}

void SimpleMidiplayerAudioProcessor::timerCallback()
{
    ownElapsedTime++;
}

juce::String SimpleMidiplayerAudioProcessor::getFillYourInfoHere() {
    return fillYourInfoHere;
}

juce::ScopedPointer<juce::Component> SimpleMidiplayerAudioProcessor::getThisWindowThingy()
{
    return thisWindowThingy;
}

juce::String SimpleMidiplayerAudioProcessor::insertMessageLog(juce::String whatToAppend)
{
    fillYourMessageHere << juce::String(whatToAppend) << "\n";
    if (tellInfoModes == 1)
        daInfoTextBox->insertTextAtCaret("\n" + juce::String(whatToAppend));

    return whatToAppend;
}

juce::String SimpleMidiplayerAudioProcessor::insertLyricLog(juce::String syllable)
{
    fillYourLyricHere << syllable;

    if (tellInfoModes == 2)
        daInfoTextBox->insertTextAtCaret(syllable);

    return syllable;
}
