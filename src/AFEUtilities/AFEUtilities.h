/*----------------------------------------------------------------------------
    Copyright 2020-2022 NXP
    SPDX-License-Identifier: BSD-3-Clause
----------------------------------------------------------------------------*/
#include <alsa/asoundlib.h>
#include <signal.h>
#ifndef _AFE_UTILITIES_
#define _AFE_UTILITIES_

extern bool tuned;
extern bool rate_shift_enable;

int rateShiftControl(int val);

#endif
