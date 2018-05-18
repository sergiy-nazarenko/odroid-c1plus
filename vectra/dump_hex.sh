#!/bin/bash

xterm -geometry 120x40 -e /usr/bin/candump can0 & 
/home/odroid/workspace/vectra/vevent $!


