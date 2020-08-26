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
#include "AudioStreamException.h"

AudioStreamException::AudioStreamException(
    const char * error_message,
    const char * error_source,
    const char * file,
    int line,
    int error_code) : std::exception(),
    _error_message(error_message),
    _source(error_source),
    _file(file),
    _line(line),
    _error_code(error_code)
{
}

const char * AudioStreamException::what(void) const noexcept
{
    return this->_error_message;
}

const char * AudioStreamException::getSource(void) const noexcept
{
    return this->_source;
}

const char * AudioStreamException::getFile(void) const noexcept
{
    return this->_file;
}

int AudioStreamException::getLine(void) const noexcept
{
    return this->_line;
}

int AudioStreamException::getErrorCode(void) const noexcept
{
    return this->_error_code;
}
