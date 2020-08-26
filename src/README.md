# Audio Front End (AFE) repository
Audio Front End (AFE) repository for solutions incorporating Voice Assistants like Alexa from Amaozn etc.

The signal gathered from the environment is noisy with echo.
To be able to filter out noise, echo etc. a set o microphone array is placed on a PCB to allow precise
signal filtering. Based on the number of microphones the input into the embedded device consists
of several microphone signals. Such a compound signal should be fed into the AFE library, which filters 
out noise, echo and other disturbances. The output from the AFE is a single channel clear audio output, 
which is in the second stage provided to the voice assistant.

The idea of the application is based around ALSA loopback device. The microphone input is redirected (written)
into the loopback. The AFE is read from this loopback device and process the signal. After the signal is
processed, the signal is written again into the loopback device and the AVS listens (reads) data from
this loopback device.

The same applies for playback. The playback signal is written into the loopback device, the AFE reads this
signal, filters it and writes it again into the loopback device.

The ALSA loopback device consists of a capture/playback loopback devices. For instace a loopback capture 
device 0,0,0 represents a Card 0, Device 0, Subdevice 0. Writing into such a device results in a signal being
copied into capture loopback device 0,1,0, which can be read by the AFE. The AFE processes this signal and
writes it back into the TODO

# How to compile source code on target
To compile the library, go into afe/src direcotry and run following command:

`g++ -Wall -g3 -O0 -o afe main.cpp AudioStream.cpp -lasound`

# How to compile source code on desktop
To be able to develop on desktop (not on target board), you need to have the
targets board linux SDK. Yocto can be used to extract the SDK for you.
After you have your SDK, the environment variables need to be set so
invoking $CXX invokes your targets board compiler instead of your Linux/WSL compiler.

## How to get Linux SDK with Yocto
TODO

## Setting environment variables
After your target board SDK has been successfully installed, you can source the
setting script. This script is located under:

`<your_sdk_path>/<sdk_revision>/`

To source the script, just execute on your command line:

`source <your_sdk_path>/<sdk_revision>/environment-setup-aarch46-poky-linux`

From now on, your global variables point to the SDKs tools.

## How to compile the afe source code
To compile the application, go into afe/src directory and run following command:

`$CXX -Wall -g3 -O0 -o afe main.cpp AudioStream.cpp -lasound`

# How to load app on board
To load the compiled application from your desktop to target board, ethernet connection is
required.
Using the scp command, you can load the image as follows:

`scp afe root@<ip_address_of_board>:<path>`

example:

`scp afe root@192.168.2.3:~/`

This will copy the binary under /home/root

- [ ] review the AudioStream class to support playback as well 
- [ ] write makefile or cmake script for building the application