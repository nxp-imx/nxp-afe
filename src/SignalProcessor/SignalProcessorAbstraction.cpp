// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020 - 2021 NXP. All rights reserved.
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
