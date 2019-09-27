#!/bin/bash
# https://wiki.odroid.com/odroid-c1/application_note/gpio/irq
# https://forum.odroid.com/viewtopic.php?f=111&t=10871
# https://www.igorkromin.net/index.php/2016/09/17/configure-dietpi-to-shut-down-after-power-button-press-on-odroidxu4/
# CONNECTION is PIN17(3.3V) to PUSH-BUTTON to PIN15
GPIO_N=108

echo $GPIO_N > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio$GPIO_N/direction

while true; do

        GPIO_VALUE=`cat /sys/class/gpio/gpio$GPIO_N/value`
        #https://wiki.archlinux.org/index.php/Allow_users_to_shutdown
        #/etc/sudoers
        #user hostname =NOPASSWD: /usr/bin/systemctl poweroff,/usr/bin/systemctl halt,/usr/bin/systemctl reboot
        if [ $GPIO_VALUE -eq 1 ]; then
                sudo systemctl reboot
        fi
        sleep 1
done