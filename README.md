# odroid-c1plus
===================================
***
Install arvdude
---------------------

sudo apt-get install build-essential bison flex automake libelf-dev libusb-1.0.0-dev libusb-dev libftdi-dev libftdi1

mkdir avrdude

cd arvdude/

wget http://download.savannah.gnu.org/releases/avrdude/avrdude-6.3.tar.gz

tar xvfz avrdude-6.3.tar.gz 

cd avrdude-6.3

**copy from here patched files**

./configure --enable-linuxgpio

make

sudo make install

