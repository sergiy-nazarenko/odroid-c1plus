#!/bin/bash

xterm -geometry 120x40 -e /usr/bin/cansniffer can0 -c & 
/home/odroid/workspace/vectra/vevent $!


