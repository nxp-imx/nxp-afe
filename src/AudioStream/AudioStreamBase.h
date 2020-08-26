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
#ifndef AUDIO_STREAM_BASE_GUARD_
#define AUDIO_STREAM_BASE_GUARD_

#include "AudioStreamException.h"

namespace AudioStreamWrapper
{

#define NOT_IMPLEMENTED_ERROR   -1

class AudioStreamBase
{
public:
    virtual
    ~AudioStreamBase(void) {};

    virtual void
    start(void) = 0;

    virtual void
    stop(bool force) = 0;

    virtual void
    close(void) = 0;

    virtual int
    recover(int err);

    virtual void
    printConfig(void);
};

} // namespace AudioStreamWrapper

#endif /* AUDIO_STREAM_BASE_GUARD_ */
