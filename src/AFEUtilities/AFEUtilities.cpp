// SPDX-License-Identifier: BSD-3-Clause
#include "AFEUtilities.h"
#include <stdlib.h>

bool tuned = false;
bool rate_shift_enable = false;

int rateShiftControl(int val)
{
	snd_ctl_t *ctl;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *value;
    int err;

	/* hw:CARD=Loopback */
    if ((err = snd_ctl_open(&ctl, "hw:CARD=Loopback", SND_CTL_NONBLOCK)) < 0)
    {
        printf("[rateShiftControl]: Unable to open control device; %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&value);

	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_id_set_name(id, "PCM Rate Shift 100000");
	snd_ctl_elem_id_set_device(id, 0);
	snd_ctl_elem_id_set_subdevice(id, 1);

	snd_ctl_elem_value_set_id(value, id);

	snd_ctl_elem_value_set_integer(value, 0, val);
	if ((err = snd_ctl_elem_write(ctl, value)) < 0)
	{
		printf("[rateShiftControl]: Unable to write value to element; %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	snd_ctl_close(ctl);
	return 0;
}
