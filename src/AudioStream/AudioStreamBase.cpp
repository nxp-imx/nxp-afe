// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020 - 2021 NXP. All rights reserved.
#include "AudioStreamBase.h"

namespace AudioStreamWrapper
{
    int
    AudioStreamBase::recover(int err)
    {
        throw AudioStreamException("Function not implemented!", "AudioStream", __FILE__, __LINE__, NOT_IMPLEMENTED_ERROR);
        return -1;
    }

    void
    AudioStreamBase::printConfig(void)
    {
        throw AudioStreamException("Function not implemented!", "AudioStream", __FILE__, __LINE__, NOT_IMPLEMENTED_ERROR);
    }

}   /* namespace AudioStream */
