// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020 - 2021 NXP. All rights reserved.
#include "SignalProcessor/SignalProcessor_Dummy.h"
#include <string>
#include <iostream>
#include <exception>

namespace SignalProcessor
{
    const std::string SignalProcessor_Dummy::_jsonConfigDescription =
    "{\n\
        \"default_config\" : {\n\
            \"sample_rate\" : -1,\n\
            \"sample_format\" : S32_LE,\n\
            \"period_size\" : 512,\n\
            \"input_channels\" : 6,\n\
            \"ref_channels\" : -1,\n\
            \"channel2output\" : 0\n\
        },\n\
        \"valid_options\" : {\n\
            \"sample_format\" : [\"string\", \"enum\", [\"S8\", \"U8\", \"S16_LE\", \"S16_BE\", \"U16_LE\", \"U16_BE\", \"S24_LE\", \"S24_BE\", \"U24_LE\", \"U24_BE\", \"S32_LE\", \"S32_BE\", \"U32_LE\", \"U32_BE\", \"FLOAT_LE\", \"FLOAT_BE\", \"FLOAT64_LE\", \"FLOAT64_BE\"]],\n\
            \"period_size\" : [\"int\", \"range\", 0, 4096,\n\
            \"input_channels\" : [\"int\", \"range\", 1, 1024],\n\
            \"channel2output\" : [\"int\", \"range\", 0, \"input_channels_max\"]\n\
        }\n}";

    SignalProcessor_Dummy::SignalProcessor_Dummy(void) : _state(DummySignalProcessorState::closed)
    {
        /* Initialize the _signalProcessorSettings to default values */
        setDefaultSettings();
    }

    int
    SignalProcessor_Dummy::openProcessor(const std::unordered_map<std::string, std::string> * settings)
    {
        int err;

        if (DummySignalProcessorState::closed != this->_state)
        {
            /* Instance is already opened. Report error as the instance needs to be closed first. */
            return -1;
        }

        /* Overwrite default settings in case the user provided his own configuration */
        if (nullptr != settings)
        {
            std::string format = settings->at("sample_format");
            snd_pcm_format_t formatValue = snd_pcm_format_value(format.c_str());
            if (SND_PCM_FORMAT_UNKNOWN != formatValue)
            {
                this->_sampleFormat = formatValue;
                /* Get the size of bytes for one sample for given format */
                this->_sampleSize = snd_pcm_format_size(this->_sampleFormat, 1);
            }
            else
            {
                closeProcessor();
                throw; /* TODO throw some meaningful exception */
            }
            this->_channel2output       = stoi(settings->at("channel2output"));
            this->_inputChannelsCount   = stoi(settings->at("input_channels"));
            this->_periodSize           = stoi(settings->at("period_size"));
        }

        /* We can check the validity of settings here */
        /* Check that the selected output channel is in range of available channels */
        if (this->_channel2output >= this->_inputChannelsCount)
        {
            throw; /* TODO throw some meaningful exception */
        }

        /* To avoid opening the signal processor multiple times, we set a "state" */
        this->_state = DummySignalProcessorState::opened;

        /* As this is a virtual signal processor, we don't have to call some internal open/init or so which
        would return a handler. So from this perspective it's all. */

        return 0;
    }

    int
    SignalProcessor_Dummy::closeProcessor(void)
    {
        /* We can deallocate internal resources here, but as we don't have any, we just
        revert all changes to mimic a fresh signal processor instace */
        if (DummySignalProcessorState::filtering != this->_state)
        {
            setDefaultSettings();
            this->_state = DummySignalProcessorState::closed;
            return 0;
        }
        return -1;
    }

    int
    SignalProcessor_Dummy::processSignal(
                const char* nChannelMicBuffer, size_t micBufferSize,
                const char* nChannelRefBuffer, size_t refBufferSize,
                char* cleanMicBuffer, size_t cleanMicBufferSize)
    {
        /* lets check, that the size of buffer matches the input settings. The dummy processor supports any buffer size,
        however, we need to check, that the provided size matches the configured. */
        size_t expectedBufferSize = this->_inputChannelsCount * this->_periodSize * this->_sampleSize;
        if (micBufferSize != expectedBufferSize)
        {
            std::cout << "input buffer size doesn't match; expected: " << expectedBufferSize << "; got: " << micBufferSize << std::endl;
            return -1;
        }
        expectedBufferSize = this->_periodSize * this->_sampleSize;
        if (cleanMicBufferSize != expectedBufferSize)
        {
            std::cout << "output buffer size doesn't match" << std::endl;
            return -2;
        }

        /* Let's process the signal - meaning copy the selected channel to output. */
        this->_state = DummySignalProcessorState::filtering;
        int shift = this->_inputChannelsCount * this->_sampleSize;

        for (int i = 0; i < this->_periodSize; i++)
        {
            memcpy(cleanMicBuffer + i * this->_sampleSize,
                nChannelMicBuffer + this->_channel2output + i * shift,
                this->_sampleSize);
        }
        this->_state = DummySignalProcessorState::opened;
        return 0;
    }

    const std::string &
    SignalProcessor_Dummy::getJsonConfigurations(void) const
    {
        return _jsonConfigDescription;
    }

    int
    SignalProcessor_Dummy::getSampleRate(void) const
    {
        return this->_sampleRate;
    }

    const char *
    SignalProcessor_Dummy::getSampleFormat(void) const
    {
        return snd_pcm_format_name(this->_sampleFormat);
    }

    int
    SignalProcessor_Dummy::getPeriodSize(void) const
    {
        return this->_periodSize;
    }

    int
    SignalProcessor_Dummy::getInputChannelsCount(void) const
    {
        return this->_inputChannelsCount;
    }

    int
    SignalProcessor_Dummy::getReferenceChannelsCount(void) const
    {
        return this->_referenceChannelsCount;
    }

    uint32_t
    SignalProcessor_Dummy::getVersionNumber(void) const
    {
        /* version 1.0.0 */
        return (uint32_t)((uint32_t)1 << 24);
    }
    /*
     *  Private section
     */
    void
    SignalProcessor_Dummy::setDefaultSettings(void)
    {
        /* As we don't filter anything, we just copy samples from input to output,
        we don't care about the sample rate meaning we support everything => -1. */
        this->_sampleRate               = -1;
        /* In general we support any format, but we need to specify which one, so we know the sample size */
        this->_sampleFormat             = SND_PCM_FORMAT_S32_LE;
        /* The default period size is 512 frames */
        this->_periodSize               = 512;
        /* The number of input channels by default is 6. */
        this->_inputChannelsCount       = 6;
        /* As we don't filter anything, we don't care about the number of reference channels,
        from this perspective we support any number of channels, as they are not used anyweay. */
        this->_referenceChannelsCount   = -1;
        /* By default, we will use the first channel as output. */
        this->_channel2output           = 0;
        /* Derive the sample size from the sample format. We store the value to no compute the
        sample size every time in our code we need it. */
        this->_sampleSize = snd_pcm_format_size(this->_sampleFormat, 1);
    }

}   /* namespace SignalProcessor */

/*
This is a mandatory interface. We need to get an instance of our class, however, we can't
export a Constructor/Destructor. So we create these C functions, which can be dynamically loaded
(their names don't get mangeled, as they are in C, not C++). These return us a pointer to our
instance, which can be used to invoke the rest of functions defined in the implementation.
*/
extern "C"  /* !!!We need to define these functions as extern "C", so their names don't get mangeled and we can load them dynamically!!! */
{
    SignalProcessor::SignalProcessorImplementation * createProcessor(void)
    {
        return new SignalProcessor::SignalProcessor_Dummy();
    }

    void destroyProcessor(SignalProcessor::SignalProcessorImplementation * processorHandle)
    {
        delete processorHandle;
    }
}