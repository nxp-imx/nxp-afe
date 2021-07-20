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

# How to execute program

1. install aloop module to support afe
`insmod snd-aloop.ko`

2. handle asound.conf. You'd better make a backup of */etc/asound.conf* 
in rootfs, and then copy *deploy_afe/asound.conf* to */etc/* to replace 
the asound.conf file there.

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
