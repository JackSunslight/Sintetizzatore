#include "WaveTable.h"



void WaveTable::prepareToPlay(double sampleRate)
{
	this->sampleRate = sampleRate;

	initializeOscillators();
}

void WaveTable::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	auto currentSample = 0;

	for (const auto midiMessage : midiMessages)
	{
		const auto midiEvent = midiMessage.getMessage();
		const auto midiEventSample = static_cast<int>(midiEvent.getTimeStamp());

		render(buffer, currentSample, midiEventSample);
		handleMidiEvent(midiEvent);

		currentSample = midiEventSample;


	}
	render(buffer, currentSample, buffer.getNumSamples());
}


void WaveTable::initializeOscillators()
{
	constexpr auto OSCILLATORS_COUNT = 128; //polifonico

	const auto waveTable = generateSineWaveTable();

	oscillators.clear();
	for (auto i = 0; i < OSCILLATORS_COUNT; ++i)
	{
		oscillators.emplace_back(waveTable, sampleRate);
	}
}

std::vector<float> WaveTable::generateSineWaveTable()
{
	constexpr auto WAVETABLE_LENGTH = 64; //fissiamo noi la lunghezza, potrebbe anche farlo l'utente

	std::vector<float> sineWaveTable(WAVETABLE_LENGTH);

	//definiamo noi stessi il pi greco 
	const auto PI = std::atanf(1.f) * 4;

	for (auto i = 0; i < WAVETABLE_LENGTH; ++i)
	{
		sineWaveTable[i] = std::sinf(2 * PI * static_cast<float>(i) / static_cast<float>(WAVETABLE_LENGTH));
	}

	return sineWaveTable;
}

void WaveTable::handleMidiEvent(const juce::MidiMessage& midiEvent)
{
	if (midiEvent.isNoteOn())
	{
		const auto oscillatorId = midiEvent.getNoteNumber();
		const auto frequency = midiNoteNumberToFrequency(oscillatorId);
		oscillators[oscillatorId].setFrequency(frequency);
	}
	else if (midiEvent.isNoteOff())
	{
		const auto oscillatorId = midiEvent.getNoteNumber();
		oscillators[oscillatorId].stop();
	}
	else if (midiEvent.isAllNotesOff())
	{
		for (auto& oscillator : oscillators)
		{
			oscillator.stop();
		}
	}
}

float WaveTable::midiNoteNumberToFrequency(int midiNoteNumber)
{
	const auto A4_FREQUENCY = 440.f;
	constexpr auto A4_NOTE_NUMBER = 69.f;
	constexpr auto SEMITONES_IN_AN_OCTAVE = 12.f;
	return A4_FREQUENCY * std::powf(2.f, (midiNoteNumber - A4_NOTE_NUMBER) / SEMITONES_IN_AN_OCTAVE);
}

void WaveTable::render(juce::AudioBuffer<float>& buffer, int startSample, int endSample)
{
	auto* firstChannel = buffer.getWritePointer(0);

	for (auto& oscillator : oscillators)
	{
		if (oscillator.isPlaying())
		{
			for (auto sample = startSample; sample < endSample; ++sample)
			{
				firstChannel[sample] += oscillator.getSample();
			}
		}
	}

	for (auto channel = 1; channel < buffer.getNumChannels(); ++channel)
	{
		std::copy(firstChannel + startSample, firstChannel + endSample, buffer.getWritePointer(channel) + startSample);
	}
}
