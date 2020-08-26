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

#ifndef AUDIO_STREAM_EXCEPTION_GUARD_
#define AUDIO_STREAM_EXCEPTION_GUARD_

class AudioStreamException : public std::exception
{
    public:
        AudioStreamException(const char * error_message, const char * error_source, const char * file, int line, int error_number);
        const char * what(void) const noexcept;
        const char * getFile(void) const noexcept;
        const char * getSource(void) const noexcept;
        int getLine(void) const noexcept;
        int getErrorCode(void) const noexcept;

    private:
        const char * _error_message;
        const char * _source;
        const char * _file;
        int _line;
        int _error_code;
};

#endif /* AUDIO_STREAM_EXCEPTION_GUARD_ */