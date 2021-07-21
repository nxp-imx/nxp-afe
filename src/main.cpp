// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020 - 2021 NXP. All rights reserved.
#include <stdio.h>
#include <string>
#include <alsa/asoundlib.h>
#include <iostream>
#include <dlfcn.h>

#include "AudioStream/AudioStream.h"
#include "SignalProcessor/SignalProcessorImplementation.h"

std::string commandUsageStr =
    "Invalid input arguments!\n" \
    "Refer to the following command:\n" \
    "./afe libdummy\n" \
    "./afe libdspc\n" \
    "./afe libfraunhofer";

enum class libraryType {
    DUMMY,
    DSPC,
    FRAUNHOFER
} libIndex;

std::unordered_map<std::string, libraryType> libraryInfo = {
	{"libdummy", libraryType::DUMMY}, 
    {"libdspc", libraryType::DSPC},
    {"libfraunhofer", libraryType::FRAUNHOFER}
};

std::string libraryName;
std::string libraryDir = "/usr/lib/nxp-afe/";

/**
 * @brief Convert S32_LE to float
 * 
 * @retval value in range <-1, 1>
 * 
 * @remark Everything below -1 and above 1 is clipped to be in range.
 */
float s32letofloat(int32_t s32Value);

using namespace AudioStreamWrapper;

static const char * playbackLoopbackInputName	= "prloop";
static snd_pcm_format_t format 					= SND_PCM_FORMAT_S32_LE;
static const StreamType acc 					= StreamType::eInterleaved;
static const int playbackOutputChannels			= 2;
static int rate 								= 16000;
static int period_size 							= 512;
static int buffer_size 							= period_size * 4;

static const snd_pcm_format_t formatPlaybackLoop = SND_PCM_FORMAT_S32_LE; 
 
static const char * playbackOutputName 	= "spk";
static const char * captureInputName 	= "mic";
static const int captureInputChannels 	= 6;
static const char * captureLoopbackOutputName 	= "cwloop";
static const int captureOutputChannels  = 1;

static const int outputStreamChannels = 1;

typedef void * (*creator)(void);
typedef void * (*destructor)(SignalProcessor::SignalProcessorImplementation * impl);

int main (int argc, char *argv[])
{
	if (argc == 2 && libraryInfo.find(argv[1]) != libraryInfo.end()) {
        libIndex = libraryInfo[argv[1]];
		switch (libIndex) {
		case libraryType::DUMMY:
			libraryName = libraryDir + "libdummyimpl.so";
			break;
		case libraryType::DSPC:
			libraryName = libraryDir + "libdspcimpl.so";
			break;
		case libraryType::FRAUNHOFER:
			libraryName = libraryDir + "libfraunhoferimpl.so";
			break;
		default:
			break;
		}
        std::cout << libraryName << std::endl;
    } else {
        std::cout << commandUsageStr << std::endl;
        exit(1);
    }
	
	//fraunhofer library only supports FLOAT_LE 
	//set format for captureInput as well as captureLoopback 
	if (libIndex == libraryType::FRAUNHOFER) {
		format = SND_PCM_FORMAT_FLOAT_LE;
	}

	//dspc library only supports buffer size 768 and rate 48000
	if (libIndex == libraryType::DSPC) {
		period_size = 768;
		buffer_size = period_size * 4;
		rate = 48000;
	}
	struct streamSettings playbackLoopbackSettings =
	{
		playbackLoopbackInputName,
		formatPlaybackLoop,
		acc,
		StreamDirection::eInput,
		playbackOutputChannels,					/* channels */
		rate, 									/* rate */
		buffer_size,							/* buffer size in frames */
		period_size,							/* period size in frames */
	};

	struct streamSettings playbackOutputSettings =
	{
		playbackOutputName,
		SND_PCM_FORMAT_S32_LE,
		acc,
		StreamDirection::eOutput,
		playbackOutputChannels,					/* channels */
		rate, 									/* rate */
		buffer_size,							/* buffer size in frames */
		period_size,							/* period size in frames */
	};

	struct streamSettings captureInputSettings =
	{
		captureInputName,
		format,
		acc,
		StreamDirection::eInput,
		captureInputChannels,
		rate,
		buffer_size,
		period_size
	};

	struct streamSettings captureLoopbackSettings = 
	{
		captureLoopbackOutputName,
		format,
		acc,
		StreamDirection::eOutput,
		captureOutputChannels,
		rate,
		buffer_size,
		period_size,
	};

	char * message;
	std::cout << "Openning " << libraryName << std::endl;
	void * library = dlopen(libraryName.c_str(), RTLD_NOW);
	message = dlerror();
	if (nullptr != message) {
		std::cout << "Opening library failed: " << message << std::endl;
		exit(1);
	}

	std::cout << "Retrieving 'createProcessor' from library...\n";
	creator createFce = (void * (*)())dlsym(library, "createProcessor");
	message = dlerror();
	if (nullptr != message) {
		std::cout << "createProcessor load: " << message << std::endl;
		exit(1);
	}

	std::cout << "Retrieving 'destroyProcessor' from library...\n";
	destructor destroyFce = (void *(*)(SignalProcessor::SignalProcessorImplementation * impl))dlsym(library, "destroyProcessor");
	message = dlerror();
	if (nullptr != message) {
		std::cout << "destroyProcessor load: " << message << std::endl;
		exit(1);
	}

	SignalProcessor::SignalProcessorImplementation * impl = (SignalProcessor::SignalProcessorImplementation *) createFce();

	std::cout << impl->getJsonConfigurations() << std::endl;

	std::cout << impl->getSampleFormat() << std::endl;
	
	std::cout << "Opening signal processor...\n";
	std::unordered_map<std::string, std::string> processorSettings = {{"sample_format", "S32_LE"}, {"channel2output", "0"}, {"input_channels", "6"}, {"period_size", "512"}};
	//impl->openProcessor(&processorSettings);
	impl->openProcessor();
	std::cout << "Signal processor opened.\n";

	int err;
	AudioStream playbackLoopbackInput;
	playbackLoopbackInput.open(playbackLoopbackSettings);
	playbackLoopbackInput.printConfig();

	AudioStream playbackOutput;
	playbackOutput.open(playbackOutputSettings);
	playbackOutput.printConfig();

	AudioStream captureInput;
	captureInput.open(captureInputSettings);
	captureInput.printConfig();

	AudioStream captureLoopbackOutput;
	captureLoopbackOutput.open(captureLoopbackSettings);
	captureLoopbackOutput.printConfig();


	int sampleSize = snd_pcm_format_width(format) / 8;
	void * buffer = calloc(period_size * playbackOutputChannels, sampleSize);

	void * captureBuffer = calloc(period_size * captureInputChannels, sampleSize);

	void * filteredBuffer = calloc(period_size * captureOutputChannels, sampleSize);
	void * cleanMicChannel;

	if (1 == outputStreamChannels)
	{
		cleanMicChannel = filteredBuffer;
	}
	else
	{
		cleanMicChannel = calloc(period_size * outputStreamChannels, sampleSize); 
	}
	
	try
	{
		err = playbackOutput.writeFrames(buffer, period_size * playbackOutputChannels * sampleSize);
		err = playbackOutput.writeFrames(buffer, period_size * playbackOutputChannels * sampleSize);
		err = playbackOutput.writeFrames(buffer, period_size * playbackOutputChannels * sampleSize);
		
		err = captureLoopbackOutput.writeFrames(buffer, period_size * captureOutputChannels * sampleSize);
		err = captureLoopbackOutput.writeFrames(buffer, period_size * captureOutputChannels * sampleSize);
		err = captureLoopbackOutput.writeFrames(buffer, period_size * captureOutputChannels * sampleSize);
	}
	catch (AudioStreamException & e)
	{
		std::cout << e.what() << std::endl;
		std::cout << e.getSource() << std::endl;
	}

	playbackLoopbackInput.start();
	captureInput.start();

	bool micSamplesReady = false;
	bool spkSamplesReady = false;
	uint32_t s32le_sample;
	float f_sample;

	try
	{

		while (1)
		{
			if (period_size == (err = captureInput.readFrames(captureBuffer, period_size * captureInputChannels * sampleSize)) && micSamplesReady != true)
			{
				micSamplesReady = true;
			}
			else
			{
				if (err < 0)
					throw AudioStreamException(snd_strerror(err), "readFrames", __FILE__, __LINE__, err);
			}

			if (period_size == (err = playbackLoopbackInput.readFrames(buffer, period_size * playbackOutputChannels * sampleSize)) && spkSamplesReady != true)
			{
				spkSamplesReady = true;

				err = playbackOutput.writeFrames(buffer, period_size * playbackOutputChannels * sampleSize);
				if (err < 0)
					throw AudioStreamException(snd_strerror(err), "writeFrames", __FILE__, __LINE__, err);
				
				//convert format if fraunhofer library used
				if (libIndex == libraryType::FRAUNHOFER) {
					for (int i = 0; i < period_size * playbackOutputChannels; i++) {
						s32le_sample = *((int *)buffer + i);
						f_sample = s32letofloat(s32le_sample);
						*((float *)buffer + i) = f_sample;
					}
				}

			}
			else
			{
				if (err < 0)
					std::cout << "Playback err: " << snd_strerror(err) << std::endl;
			}

			if ((micSamplesReady == true) && (spkSamplesReady == true))
			{
				/* Filter signal and write it to mic loopback */
				impl->processSignal(
					(char *)captureBuffer, period_size * captureInputChannels * sampleSize,
					(char *)buffer, period_size * playbackOutputChannels * sampleSize,
					(char *)filteredBuffer, period_size * outputStreamChannels * sampleSize);

				if (1 != outputStreamChannels)
				{
					for (int i = 0; i < period_size; i++)
					{
						memcpy((char *)cleanMicChannel + i * sampleSize, (char *)filteredBuffer + i * outputStreamChannels * sampleSize, sampleSize);
					}
				}

				captureLoopbackOutput.writeFrames(cleanMicChannel, period_size * captureOutputChannels * sampleSize);

				micSamplesReady = false;
				spkSamplesReady = false;
			}
		}
	}
	catch (AudioStreamException &e)
	{
		std::cout << e.what() << std::endl;
		std::cout << e.getSource() << std::endl;
		std::cout << e.getErrorCode() << std::endl;
		std::cout << e.getFile() << std::endl;
		std::cout << e.getLine() << std::endl;
	}

	impl->closeProcessor();
	destroyFce(impl);
	dlclose(library);
	return 0;
}

float s32letofloat(int32_t s32Value)
{
	float retval;
	retval = ((float)s32Value) / ((float)4294967295);
	if (retval > 1.f)
		retval = 1.f;
	if (retval < -1.f)
		retval = -1.f;
	return retval;
}