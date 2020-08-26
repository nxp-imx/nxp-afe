#include <exception>
using namespace std;

#include "AudioFrontEnd/AudioFrontEnd.h"

#include <dlfcn.h>

namespace AudioFrontEnd
{

AudioFrontEnd::AudioFrontEnd::AudioFrontEnd(void)
{
}

AudioFrontEnd::~AudioFrontEnd(void)
{
    if (nullptr != this->_signalProcessorLibrary)
    {
        unloadProcessor();
    }
}

void AudioFrontEnd::loadProcessor(std::string& path)
{
    /* We first load the library. If we succeed, we close the old library
    and replace it with a new one. Afterwards, we try to load the open/close
    functions of given processor. */
    void * tmp = dlopen(path.c_str(), RTLD_NOW);
    char * message = dlerror();
    if (nullptr != message)
    {
        /* TODO throw some meaningful error */
        throw;
    }

    if (nullptr != this->_signalProcessorLibrary)
    {
        int err = dlclose(this->_signalProcessorLibrary);
        if (0 != err)
        {
            dlclose(tmp);
            /* What do we now? */
            /*  TODO maybe throw some meaningful exception?? */
            throw;
        }
    }

    this->_signalProcessorLibrary = tmp;

    this->_sigProcCreator = (void * (*)(void))dlsym(this->_signalProcessorLibrary, "createProcessor");
    message = dlerror();
    if (nullptr != message)
    {
        _sigProcCreator = nullptr;
        dlclose(this->_signalProcessorLibrary);
        /* TODO maybe throw some meaningful exception?? */
        throw;
    }
    this->_sigProcDesctructor = (void * (*)(SignalProcessor::SignalProcessorImplementation *impl)) dlsym(this->_signalProcessorLibrary, "destroyProcessor");
    message = dlerror();
    if (nullptr != message)
    {
        _sigProcDesctructor = nullptr;
        dlclose(this->_signalProcessorLibrary);
        /* TODO maybe throw some meaningful exception?? */
        throw;
    }

    _signalProcessorImplementation = (SignalProcessor::SignalProcessorImplementation *)this->_sigProcCreator();
}

void AudioFrontEnd::unloadProcessor(void)
{
    if (nullptr != this->_signalProcessorLibrary)
    {
        this->_sigProcDesctructor(this->_signalProcessorImplementation);

        int err = dlclose(this->_signalProcessorLibrary);
        if (0 != err)
        {
            /* TODO throw some meaningful exception??? */
            throw;
        }

        this->_sigProcCreator = nullptr;
        this->_sigProcDesctructor = nullptr;
        this->_signalProcessorLibrary = nullptr;
    }
}

void AudioFrontEnd::openProcessor(std::unordered_map<std::string, std::string> * settings)
{
    if (nullptr != this->_signalProcessorImplementation)
    {
        this->_signalProcessorImplementation->openProcessor(settings);
    }
}

void AudioFrontEnd::closeProcessor(void)
{
    if (nullptr != this->_signalProcessorImplementation)
    {
        this->_signalProcessorImplementation->closeProcessor();
    }
}

std::string
AudioFrontEnd::getProcessorConfigurationSpace(void)
{
    std::string configSpace = "";
    if (nullptr != this->_signalProcessorImplementation)
    {
        configSpace = this->_signalProcessorImplementation->getJsonConfigurations();
    }

    return configSpace;
}

void AudioFrontEnd::refreshListOfSignalProcessors(std::string * path)
{

}

} /* namespace AudioFrontEnd */
