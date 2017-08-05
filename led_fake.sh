#!/bin/sh

/root/led_fake -b 2 -o 10111111

while true
do
    /root/led_fake -b 2 -o 01000000 &&
    usleep 500000 &&
    /root/led_fake -b 2 -e 01000000 &&
    usleep 500000
    done

