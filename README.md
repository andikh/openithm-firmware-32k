# openithm-firmware-32k

This is firmware for 32 key PCB version. Don't use this on 16 key as it won't work (I hardcoded this for now)
For key calibration sequence, it's start from : 

[top row]    1 3 5 7 9 11 13 15 17 19 21 23 25 27 29 31
[bottom row] 0 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30

After key calibration, there's airSensor calibration. Put away your hand from touchboard and sensor.
When touchboard flash red and yellow rapidly, wave your hand both left and right into air sensor.
Adjust the sensitivity through airsensor.h (Higher is more sensitive)

For touch.c, Put it on Your arduino folder\hardware\teensy\avr\cores\teensy3
Remember to backup your touch.c, as i only test it on my firmware so far. Check your noise through serial plotter later, And if it is worse than before just use the previous one.

Delay on airSensor is 500, you can lower them to 125 if your LED is good.
