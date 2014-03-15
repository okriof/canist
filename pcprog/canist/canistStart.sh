#!/bin/bash
cd ~/Documents/reps/gitcanist/pcprog/canist
pwd
sudo rmmod ftdi_sio
sudo chmod o+rw /dev/bus/usb/002/002
./canist

