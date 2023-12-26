/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:                  TransposerPlugin
 version:               1.0.0
 vendor:                JUCE
 website:               http://juce.com
 description:           Transposer audio plugin.

 dependencies:          juce_audio_basics, juce_audio_devices, juce_audio_formats,
                        juce_audio_plugin_client, juce_audio_processors,
                        juce_audio_utils, juce_core, juce_data_structures,
                        juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:             xcode_mac, vs2022

 moduleFlags:           JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:                  AudioProcessor
 mainClass:             Transposer

 useLocalCopy:          1

 pluginCharacteristics: pluginWantsMidiIn, pluginProducesMidiOut, pluginIsMidiEffectPlugin

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
//==============================================================================
class Transposer : public juce::AudioProcessor

                   public :
    //==============================================================================
    Transposer()
    : AudioProcessor(BusesProperties()) // add no audio buses at all
{
    addParameter(transpose = new juce::AudioParameterInt({"transpose", 0}, "Transpose", -12, 12, 0));
}

//==============================================================================
void
prepareToPlay(double sampleRate, int samplesPerBlock) override
{
    juce::ignoreUnused(samplesPerBlock);
    juce::ignoreUnused(sampleRate);
    notes.clear();
    currentNote = 0;
    lastNoteValue = -1;
}

void releaseResources() override {}

void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override
{
    // A pure MIDI plugin shouldn't be provided any audio data
    jassert(buffer.getNumChannels() == 0);
    int transpose_value = *transpose;

    buffer.clear();
    juce::MidiBuffer processedMidi;

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        const auto time = metadata.samplePosition;

        if (message.isNoteOn())
        {
            const juce::uint8 noteOnVel = message.getVelocity();
            message = juce::MidiMessage::noteOn(message.getChannel(),
                                                message.getNoteNumber() + transpose_value,
                                                (juce::uint8)noteOnVel);
        }
        else if (message.isNoteOff())
        {
            const juce::uint8 noteOffVel = message.getVelocity();
            message = juce::MidiMessage::noteOff(message.getChannel(),
                                                 message.getNoteNumber() + transpose_value,
                                                 (juce::uint8)noteOffVel);
        }

        processedMidi.addEvent(message, time);
    }

    midiMessages.swapWith(processedMidi);
}

using AudioProcessor::processBlock;

//==============================================================================
bool isMidiEffect() const override { return true; }

//==============================================================================
juce::AudioProcessorEditor *createEditor() override { return new juce::GenericAudioProcessorEditor(*this); }
bool hasEditor() const override { return true; }

//==============================================================================
const juce::String getName() const override { return "Transposer"; }

bool acceptsMidi() const override { return true; }
bool producesMidi() const override { return true; }
double getTailLengthSeconds() const override { return 0; }

//==============================================================================
int getNumPrograms() override { return 1; }
int getCurrentProgram() override { return 0; }
void setCurrentProgram(int) override {}
const juce::String getProgramName(int) override { return "None"; }
void changeProgramName(int, const juce::String &) override {}

//==============================================================================
void getStateInformation(juce::MemoryBlock &destData) override
{
    juce::MemoryOutputStream(destData, true).writeInt(*transpose);
}

void setStateInformation(const void *data, int sizeInBytes) override
{
    *transpose = juce::MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false).readInt();
}

private:
//==============================================================================
juce::AudioParameterInt *transpose;
int currentNote, lastNoteValue;
juce::SortedSet<int> notes;
//==============================================================================
JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Transposer)
}
;