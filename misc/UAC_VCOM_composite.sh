#!/bin/sh

# Originally from: https://gist.github.com/calebccff/d8c9ed5d38f1dbf1e27bb2de1da00984
# See: https://www.kernel.org/doc/Documentation/usb/gadget_configfs.txt

CONFIGFS=/sys/kernel/config/usb_gadget

if ! [ -e "$CONFIGFS" ]; then
    echo "  $CONFIGFS does not exist, skipping configfs usb gadget"
    return
fi

# Create an usb gadet configuration
mkdir $CONFIGFS/g1 || echo "  Couldn't create $CONFIGFS/g1"
cd $CONFIGFS/g1
echo "0x1fc9" > idVendor
echo "0x0101" > idProduct

# Create english (0x409) strings
mkdir strings/0x409 || echo "  Couldn't create $CONFIGFS/g1/strings/0x409"
echo "0123456789" > strings/0x409/serialnumber
echo "NXP Semiconductors" > strings/0x409/manufacturer
echo "i.MX Bar Gadget" > strings/0x409/product

# Create CDC (ACM) and USB Audio 2.0 functions. The functions can be named differently in downstream kernels.
mkdir functions/acm.GS0 \
    || echo "  Couldn't create $CONFIGFS/g1/functions/acm.GS0"
mkdir functions/uac2.UAC2Gadget \
    || echo "  Couldn't create $CONFIGFS/g1/functions/uac2.UAC2Gadget"

# Set capture and playback sample rate to 16k
echo 16000 > functions/uac2.UAC2Gadget/c_srate
echo 16000 > functions/uac2.UAC2Gadget/p_srate
# Set 32 bit audio sample size
echo 4  > functions/uac2.UAC2Gadget/c_ssize
echo 4  > functions/uac2.UAC2Gadget/p_ssize
# Set 1 channel for both capture and playback
echo 0x1 > functions/uac2.UAC2Gadget/c_chmask
echo 0x1 > functions/uac2.UAC2Gadget/p_chmask

# Create configuration instance for the gadget
mkdir configs/c.1 \
    || echo "  Couldn't create $CONFIGFS/g1/configs/c.1"
mkdir configs/c.1/strings/0x409 \
    || echo "  Couldn't create $CONFIGFS/g1/configs/c.1/strings/0x409"
echo "CDC + UAC2" > configs/c.1/strings/0x409/configuration \
    || echo "  Couldn't write configration name"

# Link the acm and uac2 instances to the configuration
ln -s functions/acm.GS0 configs/c.1 \
    || echo "  Couldn't symlink acm.GS0"
ln -s functions/uac2.UAC2Gadget configs/c.1 \
    || echo "  Couldn't symlink acm.GS0"

# Check if there's an USB Device Controller
if [ -z "$(ls /sys/class/udc)" ]; then
    echo "  No USB Device Controller available"
    return
fi

# Link the gadget instance to an USB Device Controller. This activates the gadget.
echo "$(ls /sys/class/udc)" > $CONFIGFS/g1/UDC || echo "  Couldn't write UDC"

stty -F /dev/ttyGS0 raw
stty -F /dev/ttyGS0 -echo
stty -F /dev/ttyGS0 noflsh

