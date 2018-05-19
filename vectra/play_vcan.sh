#!/bin/bash

xterm -geometry 120x40 -e canplayer vcan0=can0 -I ~/candump-2018-05-17_114414.log &
/home/odroid/workspace/vectra/vevent $!


