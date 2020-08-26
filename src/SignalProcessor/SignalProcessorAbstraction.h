/*----------------------------------------------------------------------------
    Copyright 2020 NXP

    NXP Confidential. This software is owned or controlled by NXP and may only
    be used strictly in accordance with the applicable license terms.
    By expressly accepting such terms or by downloading, installing,
    activating and/or otherwise using the software, you are agreeing that you
    have read, and that you agree to comply with and are bound by, such
    license terms. If you do not agree to be bound by the applicable license
    terms, then you may not retain, install, activate or otherwise use the
    software.
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
