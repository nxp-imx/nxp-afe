# How to compile the source code

1. fetch Linux SDK with Yocto and set up environment variables
   
2. execute makefile: `make`

3. We can find the folder named *deploy_afe* under the same directory level of
<audio-front-end>.

# What's in the *deploy_afe* folder
```
deploy_afe/
├── afe
├── asound.conf
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

# How to integrate with yocto

1. copy all library files to */usr/lib/nxp-afe/* in rootfs.

2. copy afe, asound.conf, TODO.md to */unit_tests/nxp-afe/* in rootfs 

# Preparation before the test

Here are the dtb and mic hardware needed to prepare before the test:

```
+------+-------------------+----------------------------+-----------------------------+
|      |    imx8mp         |   imx8mm                   |   imx8mn                    |
+-------------------------------------------------------------------------------------+
| dtb  |    imx8mp-evk.dtb |   imx8mm-evk-8mic-revE.dtb |   imx8mn-evk-8mic-revE.dtb  |
+-------------------------------------------------------------------------------------+
| mic  |    8mic_board     |   8mic_board               |   8mic_board                |
+------+-------------------+----------------------------+-----------------------------+

```

# How to execute program

1. install aloop module to support afe
`insmod snd-aloop.ko`

2. handle asound.conf
You'd better make a backup of */etc/asound.conf* in rootfs, 
and then copy */unit_tests/nxp-afe/asound.conf* to */etc/* to replace 
the asound.conf file there. There is a little different on imx8mp board
because the config file is asound.conf_imx8mp. Apart from copying it to */etc/*, 
the name should also be changed like:
`cp /unit_tests/nxp-afe/asound.conf_imx8mp /etc/asound.conf`

3. cd to */unit_tests/nxp-afe/* execute afe refering to the following command:
./afe libdummy
./afe libdspc
./afe libfraunhofer

# How to test

Firstly, you should put afe program into operation in the background.
Secondly, use aplay tool to playback a piece of audio through speaker.
Thirdly, use arecord tool to start record and speak to microphone soon after.
Finally, we get the audio with echo cancellation.

dummy library can not realize echo cancellation, because it is only used for
demonstration. So you will hear speech mixed with playback music.

## using dummy library
1. `./afe libdummy &`
2. `aplay S32LE16000.wav &`
3. `arecord -d10 -fS32_LE -r16000 -c1 dummy_afe_on.wav`

## using dspc library
1. `./afe libdspc &`
2. `aplay S32LE16000.wav &`
3. `arecord -d10 -fS32_LE -r16000 -c1 dspc_afe_on.wav`

## using fraunhofer library
1. `./afe libfraunhofer &`
2. `aplay S32LE16000.wav &`
3. `arecord -d10 -fFLOAT_LE -r16000 -c1 fraunhofer_afe_on.wav`

# AFE test with rpmsg soundcard
This feature depends on the 2.11 MCU SDK release. It is not available in Q4 release.

## Preparation before the test

Here are the dtb, config files, mic hardware needed to prepare before the test:

```
+------+-------------------+---------------------------------------------+
|       |    imx8mp                      |   imx8mm                      |
+-------------------------------------------------------------------------
| dtb   |    imx8mp-evk-rpmsg.dtb        |   imx8mm-evk-rpmsg-wm8524.dtb |
+-------------------------------------------------------------------------
| conf  |    asound.conf_rpmsg_imx8mp    |   asound.conf_rpmsg_imx8mm    |
+------+-------------------+---------------------------------------------+
| mic   |    8mic_board                  |   8mic_board                  |
+------+-------------------+---------------------------------------------+
```
asound.conf_rpmsg_XX need to be renamed and copied to corresponding directory.
For specific practices, refer to the introduction above.

## AFE test
The following test is the same to the above.