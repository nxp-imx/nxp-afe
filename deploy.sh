#!/bin/bash
mkdir ../deploy_afe
make
make -C src/SignalProcessor/
cp src/SignalProcessor/build/bin/* ../deploy_afe
cp build/bin/afe ../deploy_afe
make -C src/SignalProcessor/ distclean
make distclean

git checkout feature/dspc_implementation
make -C src/SignalProcessor/
cp src/SignalProcessor/build/bin/* ../deploy_afe
cp third-party/DSPC-AWELib/designer/vui_6mic_stereo.awb ../deploy_afe
make -C src/SignalProcessor/ distclean
make distclean

git checkout feature/fraunhofer_implementation
make -C src/SignalProcessor/
cp src/SignalProcessor/build/bin/* ../deploy_afe
make -C src/SignalProcessor/ distclean
make distclean

git checkout master
cp misc/asound.conf ../deploy_afe