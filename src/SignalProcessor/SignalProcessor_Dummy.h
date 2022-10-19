/*----------------------------------------------------------------------------
    Copyright 2020-2021 NXP
    SPDX-License-Identifier: BSD-3-Clause
----------------------------------------------------------------------------*/
#ifndef __SignalProcessor_Dummy_h__
#define __SignalProcessor_Dummy_h__

#include "SignalProcessor/SignalProcessorImplementation.h"
#include <string>
#include <vector>
#include <alsa/asoundlib.h>

/*------------------------------------------
Include your own header file here, if necessary:

#include "my_super_duper_signal_processor.h"

------------------------------------------*/

namespace SignalProcessor
{
    /**
     * Demonstration of signal processor implementation 
     * This class demonstrates, how a signal processor can be implemented according to given template. 
     * As this is just a dummy signal procssor, it's responsbility is only to select one out of n available 
     * channels on input and provide it as an output. No filtering is performed on the selected channel. The 
     * reference channels are ignored. 
     * This implementation demonstrates, how to possibly implement your signal processor following defined 
     * interface in the SignalProcessorImplementation base class.
     */
    class SignalProcessor_Dummy: public SignalProcessor::SignalProcessorImplementation
    {
        enum class DummySignalProcessorState
        {
            closed,
            opened,
            filtering
        };

    public:
        /**
         * Construtor, initializes internal resources
         */
        SignalProcessor_Dummy(void);
        /**
         * This set of functions comes from the base class and we need to implement them all
         * @{ */
        int
        openProcessor(const std::unordered_map<std::string, std::string> *settings = nullptr);
        int
        closeProcessor(void);
        int
        processSignal(
            const char *nChannelMicBuffer, size_t micBufferSize,
            const char *nChannelRefBuffer, size_t refBufferSize,
            char *cleanMicBuffer, size_t cleanMicBufferSize);

        int
        processSignal(
            const char *nChannelMicBuffer, size_t micBufferSize,
            const char *nChannelRefBuffer, size_t refBufferSize,
            char *cleanMicBuffer, size_t cleanMicBufferSize,
	    char* cleanRefBuffer, size_t cleanRefBufferSize);
        /*
          Returns the complete configuration space in JSON fromat
        */
        const std::string &
        getJsonConfigurations(void) const override;

        /*
          Set of functions returning the basic features supported by the signal processor.
        */
        int
        getSampleRate(void) const override;

        const char *
        getSampleFormat(void) const override;

        int
        getPeriodSize(void) const override;
        
        int
        getInputChannelsCount(void) const override;

        int
        getReferenceChannelsCount(void) const override;

        uint32_t
        getVersionNumber(void) const override;
        /** @} */

    private:
        /**
         * Here comes a dummy signal processor customization, we "need" these for the implementation
         * @{
         */
        /**
         * JSON configuration
         */
        static const std::string _jsonConfigDescription;
        /**
         * State identifier.
         */
        DummySignalProcessorState _state;
        /**
         * Size of samples in bytes.
         * 
         * @remark This is a helper parameter derived out of _sampleFormat variable
         */
        size_t _sampleSize;
        /**
         * Selected sample rate.
         */
        int _sampleRate;
        /**
         * Selected sample format.
         * 
         * @remark The number should correspond to ALSA formats.
         */
        snd_pcm_format_t _sampleFormat = SND_PCM_FORMAT_S32_LE; // the signal processor has not been opened, report -1
        /**
         * Selected period size.
         */
        int _periodSize;
        /**
         * Selected input channels count.
         */
        int _inputChannelsCount;
        /**
         * Selected reference channels count.
         */
        int _referenceChannelsCount;
        /**
         * Input channel selected as output.
         * 
         * @warning Must be in range of _inputChannelsCount indexed from 0 to (_inputChannelsCount - 1)
         */
        int _channel2output;
        /** @} */

        /**
         * This set of functions comes from the base class and we need to implement them all
         * @{ */
        /**
         * Sets the signal processors settings to default values.
         *  
         * @remark Select always the first channel to sent to output. 
         */
        void
        setDefaultSettings(void);
        /** @} */
    };
}

#endif
