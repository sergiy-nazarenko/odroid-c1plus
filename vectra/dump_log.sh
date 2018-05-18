#!/bin/bash

xterm -geometry 120x40 -e /usr/bin/candump can0 -l & 
/home/odroid/workspace/vectra/vevent $!


