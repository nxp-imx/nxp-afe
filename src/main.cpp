// Copyright 2020-2021 NXP
// SPDX-License-Identifier: BSD-3-Clause
#include <stdio.h>
#include <string>
#include <alsa/asoundlib.h>
#include <iostream>
#include <dlfcn.h>
#include <pthread.h>
#include <math.h>

#define THREAD_SLEEP_DIVISOR 	 (4U)
#define ONE_MILLION 			(1000000U)

#include "AudioStream/AudioStream.h"
#include "SignalProcessor/SignalProcessorImplementation.h"
#include "AFEUtilities/AFEUtilities.h"

std::string commandUsageStr =
    "Invalid input arguments!\n" \
    "Refer to the following command:\n" \
    "./afe libdummy\n" \
    "./afe libdspc\n" \
    "./afe libfraunhofer\n" \
    "./afe libvoiceseekerlight\n" \
    "./afe libconversa\n" \
    "./afe <lib> <reference_signal>";

enum class libraryType {
    DUMMY,
    DSPC,
    FRAUNHOFER,
    VOICESEEKERLIGHT,
    CONVERSA
} libIndex;

std::unordered_map<std::string, libraryType> libraryInfo = {
	{"libdummy", libraryType::DUMMY}, 
	{"libdspc", libraryType::DSPC},
	{"libfraunhofer", libraryType::FRAUNHOFER},
	{"libvoiceseekerlight", libraryType::VOICESEEKERLIGHT},
	{"libconversa", libraryType::CONVERSA}
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

void *thread_playback_function(void *arg);
void *thread_capture_function(void *arg);
pthread_mutex_t playback_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t capture_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_var_p = PTHREAD_COND_INITIALIZER;

using namespace AudioStreamWrapper;

static const char * playbackLoopbackInputName	= "prloop";
static snd_pcm_format_t format 			= SND_PCM_FORMAT_S32_LE;
static const StreamType acc 			= StreamType::eInterleaved;
static int playbackOutputChannels		= 2;
static int rate 				= 16000;
static int period_size 				= 512;
static int buffer_size 				= period_size * 4;

static const snd_pcm_format_t formatPlaybackLoop = SND_PCM_FORMAT_S32_LE; 
 
static const char * playbackOutputName 		= "spk";
static const char * captureInputName 		= "mic";
static int captureInputChannels 		= 6;
static const char * captureLoopbackOutputName 	= "cwloop";
static const int captureOutputChannels 	 	= 1;

static const int outputStreamChannels 		= 1;

static useconds_t periodSizeInUs = 0;
static useconds_t sleepTimeInUs = 0;

typedef void * (*creator)(void);
typedef void * (*destructor)(SignalProcessor::SignalProcessorImplementation * impl);

static const int MAX_STR_SIZE = 32;

static AudioStream playbackLoopbackInput;
static AudioStream playbackOutput;
static AudioStream captureInput;
static AudioStream captureLoopbackOutput;

bool micSamplesReady = false;
bool spkSamplesReady = false;
bool thread_exit = false;

void * buffer;
void * captureBuffer;
void * filteredBuffer;
void * cleanMicChannel;
void * convertedBuffer;
void * refOutBuffer;	/* Output from algorithm for playback */

int sampleSize;

int main (int argc, char *argv[])
{
	bool restart_pipeline = false;
	std::unordered_map<std::string, std::string> processorSettings = {{"sample_format", "S32_LE"}, {"channel2output", "0"}, {"input_channels", "6"}, {"period_size", "512"}};

	if (argc >= 2 && libraryInfo.find(argv[1]) != libraryInfo.end()) {
		libIndex = libraryInfo[argv[1]];
		switch (libIndex) {
		case libraryType::DUMMY:
			libraryName = libraryDir + "libdummyimpl.so";
			break;
		case libraryType::DSPC:
			libraryName = libraryDir + "libdspcimpl.so";
			//dspc library only supports buffer size 768 and rate 48000
			period_size = 768;
			buffer_size = period_size * 4;
			rate = 48000;
			processorSettings = {{"sample_format", "S32_LE"},
					     {"channel2output", "0"},
					     {"input_channels", "6"},
					     {"period_size", "768"}};
			break;
		case libraryType::FRAUNHOFER:
			libraryName = libraryDir + "libfraunhoferimpl.so";
			//fraunhofer library only supports FLOAT_LE
			//set format for captureInput as well as captureLoopback
			format = SND_PCM_FORMAT_FLOAT_LE;
			processorSettings = {{"sample_format", "FLOAT_LE"},
					     {"channel2output", "0"},
					     {"input_channels", "6"},
					     {"period_size", "512"}};
			break;
		case libraryType::VOICESEEKERLIGHT:
			libraryName = libraryDir + "libvoiceseekerlight.so";
			period_size = 800;
			buffer_size = period_size * 4;
			captureInputChannels = 4;
			processorSettings = {{"sample_format", "S32_LE"},
					     {"channel2output", "0"},
					     {"input_channels", "4"},
					     {"period_size", "800"}};
			break;
		case libraryType::CONVERSA:
			libraryName = libraryDir + "libconversa.so";
			period_size = 192;
			buffer_size = period_size * 4;
			captureInputChannels = 4;
			processorSettings = {{"sample_format", "S32_LE"},
					     {"channel2output", "0"},
					     {"input_channels", "4"},
					     {"period_size", "192"}};

			restart_pipeline = true;
			break;
		default:
			break;
		}

        if(argc == 3) {
            if(strlen(argv[2]) > MAX_STR_SIZE) {
                std::cout << "The device name is too long" << std::endl;
                exit(1);
            }

            playbackLoopbackInputName = argv[2];
            playbackOutputName = "";
        }

		std::cout << libraryName << std::endl;
	} else {
		std::cout << commandUsageStr << std::endl;
		exit(1);
	}
	
	struct streamSettings playbackLoopbackSettings =
	{
		playbackLoopbackInputName,
		formatPlaybackLoop,
		acc,
		StreamDirection::eInput,
		playbackOutputChannels,					/* channels */
		rate, 							/* rate */
		buffer_size,						/* buffer size in frames */
		period_size,						/* period size in frames */
		false,
	};

	struct streamSettings playbackOutputSettings =
	{
		playbackOutputName,
		SND_PCM_FORMAT_S32_LE,
		acc,
		StreamDirection::eOutput,
		playbackOutputChannels,					/* channels */
		rate, 							/* rate */
		buffer_size,						/* buffer size in frames */
		period_size,						/* period size in frames */
		false,
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
		period_size,
		false,
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
		false,
	};

	int err;
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

	pthread_t thread_playback;
	pthread_t thread_capture;
	pthread_attr_t tattr;
	pthread_t tid;
	sched_param param;;
	pid_t pid;

	err = pthread_attr_init(&tattr);
	if (err < 0)
	{
		std::cout << "thread attr init failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	err = pthread_attr_setschedpolicy(&tattr, SCHED_RR);
	if (err < 0)
	{
		std::cout << "thread attr set policy failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	err = pthread_attr_getschedparam(&tattr, &param);
	if (err < 0)
	{
		std::cout << "thread attr get param failed" << std::endl;
		exit(EXIT_FAILURE);
	}
	param.sched_priority = 99;
	err = pthread_attr_setschedparam(&tattr, &param);
	if (err < 0)
	{
		std::cout << "thread attr set param failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	pid = getpid();
	err = sched_setscheduler(pid, SCHED_RR, &param);
	if (err < 0)
	{
		std::cout << "set main process scheduler failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	SignalProcessor::SignalProcessorImplementation * impl = (SignalProcessor::SignalProcessorImplementation *) createFce();

    if (impl == NULL) {
        exit(1);
    }

	std::cout << "Opening signal processor...\n";

	if (impl->getPeriodSize()) {
		period_size = impl->getPeriodSize();
		buffer_size = period_size * 4;

		processorSettings.at("period_size") = std::to_string(period_size);

		playbackLoopbackSettings.periodSizeFrames = period_size;
		playbackLoopbackSettings.bufferSizeFrames = buffer_size;

		playbackOutputSettings.periodSizeFrames = period_size;
		playbackOutputSettings.bufferSizeFrames = buffer_size;

		captureInputSettings.periodSizeFrames = period_size;
		captureInputSettings.bufferSizeFrames = buffer_size;

		captureLoopbackSettings.periodSizeFrames = period_size;
		captureLoopbackSettings.bufferSizeFrames = buffer_size;
	}

	if (impl->getReferenceChannelsCount() > 0) {
		playbackOutputChannels = impl->getReferenceChannelsCount();
		playbackLoopbackSettings.channels = playbackOutputChannels;
		playbackOutputSettings.channels = playbackOutputChannels;
	}

	switch (libIndex) {
	case libraryType::DUMMY:
	case libraryType::DSPC:
	case libraryType::FRAUNHOFER:
		impl->openProcessor();
		break;
	case libraryType::VOICESEEKERLIGHT:
		impl->openProcessor(&processorSettings);
		break;
	case libraryType::CONVERSA:
		impl->openProcessor(&processorSettings);
		playbackLoopbackSettings.debug_info_enable = true;
		playbackOutputSettings.debug_info_enable = true;
		captureInputSettings.debug_info_enable = true;
		captureLoopbackSettings.debug_info_enable = true;
		break;
	default:
		break;
	}

	std::cout << "Signal processor opened.\n";

	sampleSize = snd_pcm_format_width(format) / 8;
	buffer = calloc(period_size * playbackOutputChannels, sampleSize);
	refOutBuffer = calloc(period_size * playbackOutputChannels, sampleSize);

	captureBuffer = calloc(period_size * captureInputChannels, sampleSize);

	filteredBuffer = calloc(period_size * captureOutputChannels, sampleSize);

	if (libIndex == libraryType::FRAUNHOFER)
	{
		convertedBuffer = calloc(period_size * playbackOutputChannels, sampleSize);
	}
	else
	{
		convertedBuffer = buffer;
	}
	if (1 == outputStreamChannels)
	{
		cleanMicChannel = filteredBuffer;
	}
	else
	{
		cleanMicChannel = calloc(period_size * outputStreamChannels, sampleSize); 
	}

again:
	playbackLoopbackInput.open(playbackLoopbackSettings);
	playbackLoopbackInput.printConfig();

    try
    {
        playbackOutput.open(playbackOutputSettings);
    }
    catch (AudioStreamException &e)
    {
        if(e.getErrorCode() != (int)StreamErrors::eStreamIsClose && argc != 3) {
            std::cout << argc;
            std::cout << e.what();
            throw;
        }
    }

	playbackOutput.printConfig();

	captureInput.open(captureInputSettings);
	captureInput.printConfig();

	captureLoopbackOutput.open(captureLoopbackSettings);
	captureLoopbackOutput.printConfig();

	playbackLoopbackInput.start();
	captureInput.start();

	pthread_mutex_init(&playback_lock, NULL);
	pthread_mutex_init(&capture_lock, NULL);
	pthread_cond_init(&cond_var, NULL ) ;
	pthread_cond_init(&cond_var_p, NULL ) ;
	thread_exit = false;
	spkSamplesReady = false;
	micSamplesReady = false;

	/* Compute how often the audio buffers will need to be read or written */
	periodSizeInUs = useconds_t(round( (1/(float)rate) * (float)period_size * ONE_MILLION ));
	/* For the audio threads usleep call.*/
	sleepTimeInUs = periodSizeInUs/(useconds_t THREAD_SLEEP_DIVISOR);

	if( (0 == sleepTimeInUs) || ( periodSizeInUs < sleepTimeInUs) )
	{
		std::cout << "Warning: Computed thread sleep time is: " << sleepTimeInUs <<
		" . Performance could be affected. " << std::endl;
	}
	

	err = pthread_create(&thread_playback, &tattr, thread_playback_function, NULL);
	if (err < 0)
	{
		std::cout << "playback thread create failed" << std::endl;
		exit(EXIT_FAILURE);
	}
	err = pthread_create(&thread_capture, &tattr, thread_capture_function, NULL);
	if (err < 0)
	{
		std::cout << "capture thread create failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	/*
	 * Enable playbackOutput late, when the timer source of loopback is from it,
	 * It can help to trigger the period complete.
	 */
	try
	{
		err = captureLoopbackOutput.writeFrames(buffer, period_size * captureOutputChannels * sampleSize);
		err = captureLoopbackOutput.writeFrames(buffer, period_size * captureOutputChannels * sampleSize);
		err = captureLoopbackOutput.writeFrames(buffer, period_size * captureOutputChannels * sampleSize);
		err = captureLoopbackOutput.writeFrames(buffer, period_size * captureOutputChannels * sampleSize);
		err = playbackOutput.writeFrames(buffer, period_size * playbackOutputChannels * sampleSize);
		err = playbackOutput.writeFrames(buffer, period_size * playbackOutputChannels * sampleSize);
		err = playbackOutput.writeFrames(buffer, period_size * playbackOutputChannels * sampleSize);
		err = playbackOutput.writeFrames(buffer, period_size * playbackOutputChannels * sampleSize);
	}
	catch (AudioStreamException & e)
	{
		std::cout << e.what() << std::endl;
		std::cout << e.getSource() << std::endl;
	}

	try
	{

		while (1)
		{

			if (restart_pipeline && (playbackOutput.recover_count() || playbackLoopbackInput.recover_count() ||
					captureInput.recover_count() || captureLoopbackOutput.recover_count())) {
				thread_exit = true;
				break;
			}

			pthread_mutex_lock(&capture_lock);
			while (micSamplesReady != true)
			{
				pthread_cond_wait(&cond_var, &capture_lock);
			}

			/*
			 * After resume from suspend, data from mic should be processed as fast as possible,
			 * or there'll be latency in wake word detection.
			 */
			if (!(tuned && (libIndex == libraryType::VOICESEEKERLIGHT))) {
				pthread_mutex_lock(&playback_lock);
				while (spkSamplesReady != true)
				{
					pthread_cond_wait(&cond_var_p, &playback_lock);
				}
			}

			if (libIndex == libraryType::FRAUNHOFER) {
				uint32_t s32le_sample;
				float f_sample;
				memcpy(convertedBuffer, buffer, period_size * playbackOutputChannels * sampleSize);
				for (int i = 0; i < period_size * playbackOutputChannels; i++) {
					s32le_sample = *((int *)convertedBuffer + i);
					f_sample = s32letofloat(s32le_sample);
					*((float *)convertedBuffer + i) = f_sample;
				}
			}
			if (libIndex == libraryType::CONVERSA)
				impl->processSignal(
					(char *)captureBuffer, period_size * captureInputChannels * sampleSize,
					(char *)convertedBuffer, period_size * playbackOutputChannels * sampleSize,
					(char *)filteredBuffer, period_size * outputStreamChannels * sampleSize,
					(char *)refOutBuffer, period_size * outputStreamChannels * sampleSize);
			else
				impl->processSignal(
					(char *)captureBuffer, period_size * captureInputChannels * sampleSize,
					(char *)convertedBuffer, period_size * playbackOutputChannels * sampleSize,
					(char *)filteredBuffer, period_size * outputStreamChannels * sampleSize);

			if (1 != outputStreamChannels)
			{
				for (int i = 0; i < period_size; i++)
				{
					memcpy((char *)cleanMicChannel + i * sampleSize, (char *)filteredBuffer + i * outputStreamChannels * sampleSize, sampleSize);
				}
			}

			micSamplesReady = false;
			spkSamplesReady = false;
			if (!(tuned && (libIndex == libraryType::VOICESEEKERLIGHT)))
				pthread_mutex_unlock(&playback_lock);
			pthread_mutex_unlock(&capture_lock);

			captureLoopbackOutput.writeFrames(cleanMicChannel, period_size * captureOutputChannels * sampleSize);

			if (libIndex == libraryType::CONVERSA)
				playbackOutput.writeFrames(refOutBuffer, period_size * playbackOutputChannels * sampleSize);
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

	pthread_join(thread_playback, NULL);
	pthread_join(thread_capture, NULL);
	pthread_mutex_destroy(&capture_lock);
	pthread_mutex_destroy(&playback_lock);
	pthread_cond_destroy(&cond_var);
	pthread_cond_destroy(&cond_var_p);

	playbackLoopbackInput.close();
	playbackOutput.close();
	captureLoopbackOutput.close();
	captureInput.close();

	/* restart whole pipeline*/
	if (restart_pipeline)
		goto again;

	if (libIndex == libraryType::FRAUNHOFER)
		free(convertedBuffer);
	if (1 != outputStreamChannels)
		free(cleanMicChannel);

	free(filteredBuffer);
	free(captureBuffer);
	free(refOutBuffer);
	free(buffer);

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

void *thread_playback_function(void *arg)
{
	int err = 0;
	while (1)
	{
		if (playbackLoopbackInput.availFrames() < period_size || spkSamplesReady)
		{
			usleep(sleepTimeInUs);
		}
		else
		{
			pthread_mutex_lock(&playback_lock);
			if (period_size == (err = playbackLoopbackInput.readFrames(buffer, period_size * playbackOutputChannels * sampleSize)))
			{
				spkSamplesReady = true;
				pthread_mutex_unlock(&playback_lock);
				pthread_cond_signal(&cond_var_p);
				if (libIndex != libraryType::CONVERSA) {
                    try 
                    {
                        err = playbackOutput.writeFrames(buffer, period_size * playbackOutputChannels * sampleSize);
                        if (err < 0)
                            throw AudioStreamException(snd_strerror(err), "writeFrames", __FILE__, __LINE__, err);
                    } catch (AudioStreamException &e) {
                        if(e.getErrorCode() != (int)StreamErrors::eStreamIsClose)
                            throw AudioStreamException(snd_strerror(err), "writeFrames", __FILE__, __LINE__, err);
                    }
				}
			}
			else
			{
				pthread_mutex_unlock(&playback_lock);
				if (err < 0)
					throw AudioStreamException(snd_strerror(err), "writeFrames", __FILE__, __LINE__, err);
			}
		}

		if (thread_exit)
			break;
	}
	pthread_exit(NULL);
}

void *thread_capture_function(void *arg)
{
	int err = 0;
	while (1)
	{
		if (captureInput.availFrames() < period_size || micSamplesReady)
		{
			usleep(sleepTimeInUs);
		}
		else
		{
			pthread_mutex_lock(&capture_lock);
			if (period_size == (err = captureInput.readFrames(captureBuffer, period_size * captureInputChannels * sampleSize)))
			{
				micSamplesReady = true;
				pthread_mutex_unlock(&capture_lock);
				pthread_cond_signal(&cond_var);
			}
			else
			{
				pthread_mutex_unlock(&capture_lock);
				if (err < 0)
					throw AudioStreamException(snd_strerror(err), "readFrames", __FILE__, __LINE__, err);
			}
		}

		if (thread_exit)
			break;
	}
	pthread_exit(NULL);
}
