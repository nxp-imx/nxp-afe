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

#include "SignalProcessor/SignalProcessorAbstraction.h"
#include "SignalProcessor/SignalProcessorImplementation.h"

namespace SignalProcessor
{

    SignalProcessorAbstraction::SignalProcessorAbstraction(SignalProcessorImplementation *implementation)
    {
        setImplementation(implementation);
    }

    int SignalProcessorAbstraction::setImplementation(SignalProcessorImplementation *implementation)
    {
        this->_implementation = implementation;
        return 0;
    }

    int
    SignalProcessorAbstraction::openProcessor(const void * settings)
    {
        return this->_implementation->openProcessor(settings);
    }

    int
    SignalProcessorAbstraction::processSignal(
            const char * nChannelMicBuffer, size_t micBuffer,
            const char * nChannelRefBuffer, size_t refBufferSize,
            char * cleanMicBuffer, size_t cleanMicBufferSize)
    {
        return this->_implementation->processSignal(
            nChannelMicBuffer, micBuffer,
            nChannelRefBuffer, refBufferSize,
            cleanMicBuffer, cleanMicBufferSize);
    }

    int
    SignalProcessorAbstraction::closeProcessor(void)
    {
        return this->_implementation->closeProcessor();
    }
}   /* namespace SignalProcessor */
