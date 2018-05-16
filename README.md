# odroid-c1plus
===================================

Install WiringPi
------------------
git clone https://github.com/hardkernel/WiringPi

cd WiringPi/

./build

***
Install arvdude
---------------------

sudo apt-get install build-essential bison flex automake libelf-dev libusb-1.0.0-dev libusb-dev libftdi-dev libftdi1

sudo apt-get install arduino

mkdir avrdude

cd arvdude/

wget http://download.savannah.gnu.org/releases/avrdude/avrdude-6.3.tar.gz

tar xvfz avrdude-6.3.tar.gz 

cd avrdude-6.3

**copy from here patched files**

./configure --enable-linuxgpio

make

sudo make install



***
Links
---------------------------------------
http://www.hardkernel.com/main/products/prdt_info.php?g_code=G147563061546
https://github.com/garthvh/pitftmenu
https://github.com/Afterglow/arduino-gmlan
https://blog.dimitarmk.com/2017/02/05/gmlan-sniffing-arduino-mcp2515/
https://www.drive2.ru/l/474473078441641818/
http://pinoutguide.com/CarElectronics/car_obd2_pinout.shtml
https://www.youtube.com/watch?v=xEt6RGqBfX4&t=31s
https://www.youtube.com/watch?v=yOIXnQCXnhg&feature=youtu.be






