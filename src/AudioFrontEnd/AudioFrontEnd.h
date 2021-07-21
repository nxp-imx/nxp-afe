/*----------------------------------------------------------------------------
    SPDX-License-Identifier: BSD-3-Clause
    Copyright (c) 2020 - 2021 NXP. All rights reserved.
----------------------------------------------------------------------------*/
#include <exception>

#ifndef _AudioFrontEnd_
#define _AudioFrontEnd_

#include "AudioStream/AudioStream.h"
#include "SignalProcessor/SignalProcessorAbstraction.h"
#include "SignalProcessor/SignalProcessorImplementation.h"
#include <string>


namespace AudioFrontEnd
{
    class AudioFrontEnd
    {
        public:
            AudioFrontEnd(void);
            ~AudioFrontEnd(void);
            void loadProcessor(std::string& path);
            void unloadProcessor(void);
            void openProcessor(std::unordered_map<std::string, std::string> * settings);
            void closeProcessor(void);
            std::string getProcessorConfigurationSpace(void);
            void refreshListOfSignalProcessors(std::string* path);
        private:
            /**
             * The path where to look at for signal processor implementation librariers
             */
            const std::string defaultSignalProcessorImplementationsPath;
            /**
             * Stream representing physical input from microphones
             */
            AudioStreamWrapper::AudioStream inputCaptureStream;
            /**
             * Stream representing virtual output for a single microphone channel - loopback
             */
            AudioStreamWrapper::AudioStream outputCaptureStream;
            /**
             * Stream representing virtual input for playback - loopback
             */
            AudioStreamWrapper::AudioStream inputPlaybackStream;
            /**
             * Stream representing physical output to speakers
             */
            AudioStreamWrapper::AudioStream outputPlaybackStream;
            /**
             * Signal processor
             */
            SignalProcessor::SignalProcessorImplementation * _signalProcessorImplementation = nullptr;
            /**
             * List of paths for all libraries
             */
            std::vector<std::string> availableProcessorsPaths;
            /**
             * Handle to signal processor library
             */
            void * _signalProcessorLibrary = nullptr;
            /**
             * Signal processor constructor
             */
            void * (*_sigProcCreator)(void) = nullptr;
            /**
             * Signal processor destructor
             */
            void * (*_sigProcDesctructor)(SignalProcessor::SignalProcessorImplementation * impl) = nullptr;
    };
}

#endif /* _AudioFrontEnd_ header guard */
