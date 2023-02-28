#!/bin/sh

modprobe snd-aloop
sh UAC_VCOM_composite.sh
#Edit volume of wm8960audio accordig to your needs
amixer -c wm8960audio set Headphone 90%
amixer -c wm8960audio set Playback 90%
