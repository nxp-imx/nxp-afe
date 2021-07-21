/*----------------------------------------------------------------------------
    SPDX-License-Identifier: BSD-3-Clause
    Copyright (c) 2020 - 2021 NXP. All rights reserved.
----------------------------------------------------------------------------*/
#include <exception>
using namespace std;

#ifndef __SignalProcessor__SignalProcessorAbstraction_h__
#define __SignalProcessor__SignalProcessorAbstraction_h__

// #include "AudioFrontEnd/AudioFrontEnd.h"
#include "SignalProcessor/SignalProcessorImplementation.h"

namespace SignalProcessor
{
    class SignalProcessorAbstraction
    {
        public:
            SignalProcessorAbstraction(SignalProcessor::SignalProcessorImplementation *signalProcessor);
            virtual int
            openProcessor(const void * settings);
            virtual int
            processSignal(
                const char * nChannelMicBuffer, size_t micBuffer,
                const char * nChannelRefBuffer, size_t refBufferSize,
                char * cleanMicBuffer, size_t cleanMicBufferSize);
            virtual int
            closeProcessor(void);
            virtual int
            setImplementation(SignalProcessor::SignalProcessorImplementation *implementation);

        protected:
            SignalProcessor::SignalProcessorImplementation* _implementation;
    };
}

#endif
