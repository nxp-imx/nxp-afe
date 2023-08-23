// Copyright 2020-2021 NXP
// SPDX-License-Identifier: BSD-3-Clause
#include "AudioStreamException.h"
#include <string>
#include <cstring>

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
    std::string err = this->_file + std::string(" ") + std::to_string(this->_line);
    err = err + " " + this->_error_message;
    char *msg = new char [err.length() + 1];
    std::strcpy(msg, err.c_str());

    return msg;
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
