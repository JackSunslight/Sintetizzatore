//#include "WaveTableOscillator.h"
#include <vector>
#include <cmath>
#include "../../Source/WaveTableOscillator.h"


WaveTableOscillator::WaveTableOscillator(std::vector<float> waveTable, double sampleRate)
	: waveTable{ std::move(waveTable) }, sampleRate{ sampleRate }
{

}

void WaveTableOscillator::setFrequency(float frequency)
{
	indexIncrement = frequency * static_cast<float>(waveTable.size()) / static_cast<float>(sampleRate);
}

float WaveTableOscillator::getSample()
{
	const auto sample = interpolateLinearly();
	index += indexIncrement;
	index = std::fmod(index, static_cast<float>(waveTable.size()));
	return sample;
}

void WaveTableOscillator::stop()
{
	index = 0.f;
	indexIncrement = 0.f;
}

bool WaveTableOscillator::isPlaying()
{
	return indexIncrement != 0.f;
}

float WaveTableOscillator::interpolateLinearly()
{
	const auto truncatedIndex = static_cast<int>(index);
	const auto nextIndex = (truncatedIndex + 1) % static_cast<int>(waveTable.size());

	const auto nextIndexWeight = index - static_cast<float>(truncatedIndex);
	const auto truncatedIndexWeight = 1.f - nextIndexWeight;

	return truncatedIndexWeight * waveTable[truncatedIndex] + nextIndexWeight * waveTable[nextIndex];

}



