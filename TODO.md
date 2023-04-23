# How to compile the source code

1. Fetch Linux SDK with Yocto and set up environment variables

2. Execute makefile: `make`

3. We can find the folder named `deploy_afe` under the same directory level of
<audio-front-end>.

# What's in the *deploy_afe* folder
```
deploy_afe/
├── afe
├── asound.conf_<BOARD> (BOARDS: imx8mm, imx8mp, imx8mp_revb4, imx93)
├── libdummyimpl.so
├── libdummyimpl.so.1.0
└── TODO.md
```
- afe:
  NXP audio front end wrapper
- asound.conf:
  Alsa configuration file
- libdummyimpl.so*:
  a dummy library for demonstration
- TODO.md:
  a user guide

# How to integrate with Yocto

1. Copy all libraries files to `/usr/lib/nxp-afe/` in rootfs.

2. Copy `afe`, `asound.conf_<BOARD>`, `TODO.md` to `/unit_tests/nxp-afe/` in rootfs.

# Preparation before the test

Here are the dtb and mic hardware needed to prepare before the test:

## BOARD: i.MX8MM-EVK

DTB: imx8mm-evk-8mic-revE.dtb / imx8mm-evk-rpmsg-wm8524.dtb / imx8mm-evk-rpmsg-wm8524-lpv.dtb

Mic Hardware: 8mic_board

## BOARD: i.MX8MP-EVK

DTB: imx8mp-evk.dtb / imx8mp-evk-rpmsg.dtb / imx8mp-evk-rpmsg-lpv.dtb

Mic Hardware: 8mic_board

(Here if you want to use imx8mp-evk.dtb and your board version is revA3 or later versions but
before revB4, please use imx8mp-evk-revA3-8mic-revE.dtb. If your borard version is revB4 or
later versions, please refer to BOARD: i.MX8MP-EVK-REVB4)

## BOARD: i.MX8MP-EVK-REVB4

DTB: imx8mp-evk-revb4-8mic-revE.dtb / imx8mp-evk-revb4-rpmsg.dtb / imx8mp-evk-revb4-rpmsg-lpv.dtb

Mic Hardware: 8mic_board

## BOARD: i.MX93-11x11-EVK

DTB: imx93-11x11-evk.dtb / imx93-11x11-evk-rpmsg.dtb / imx93-11x11-evk-rpmsg-lpv.dtb

Mic Hardware: PDM mic on board

# How to execute program

1. Install aloop module to support `afe`
`sudo modprobe snd-aloop`

1. Handle `asound.conf`
You'd better make a backup of `/etc/asound.conf` in rootfs,
and then copy `/unit_tests/nxp-afe/asound.conf_<BOARD>` to `/etc/` to replace
the `asound.conf` file. Take the asound that applies on your board. For
example: If you are on and i.MX8MP-EVK the config file is `asound.conf_imx8mp`.
Apart from copying it to `/etc/`, the name should also be changed like:
`cp /unit_tests/nxp-afe/asound.conf_imx8mp /etc/asound.conf`

3. cd to `/unit_tests/nxp-afe/` execute `afe` referring to the following command:
`./afe libdummy`
`./afe libdspc`
`./afe libfraunhofer`
`./afe libvoiceseekerlight`
`./afe libconversa`

# How to test
> This chapter explain how to run the diferents libraries the AFE support. For
more information please visit the repos of the given libraries/programs.

Firstly, you should put VoiceSpot application (now called `voice_ui_app`)
into operation in the background this step is only required when the VoiceSeekerLight
library is used. Secondly, you should put `afe` program into operation in the background.
Thirdly, use `aplay` tool to playback a piece of audio through speaker.
Fourthly, use `arecord` tool to start record and speak to microphone soon after.
Finally, we get the audio with echo cancellation.

dummy library can not realize echo cancellation, because it is only used for
demonstration. So you will hear speech mixed with playback music.

## Using dummy library
1. `./afe libdummy &`
2. `aplay S32LE16000.wav &`
3. `arecord -d10 -fS32_LE -r16000 -c1 dummy_afe_on.wav`

## Using DSPC library
1. `./afe libdspc &`
2. `aplay S32LE16000.wav &`
3. `arecord -d10 -fS32_LE -r16000 -c1 dspc_afe_on.wav`

## Using Fraunhofer library
1. `./afe libfraunhofer &`
2. `aplay S32LE16000.wav &`
3. `arecord -d10 -fFLOAT_LE -r16000 -c1 fraunhofer_afe_on.wav`

## Using VoiceSeekerLight library with Voicespot
1. `./voice_ui_app &`
2. `./afe libvoiceseekerlight &`
3. `aplay S32LE16000.wav &`
4. `arecord -d10 -fS32_LE -r16000 -c1 voiceseeker_afe_on.wav`

## Using VoiceSeeker library without Voicespot by modifing the Config.ini
1. `./afe libvoiceseekerlight &`
2. `aplay S32LE16000.wav &`
3. `arecord -d10 -fS32_LE -r16000 -c1 voiceseeker_afe_on.wav`

## Using Conversa library
1. `./UAC_VCOM_composite.sh`
2. `./afe libconversa &`
3. `./afe_uac &`
4. `Connect the Conversa Tuning Tool from PC`
