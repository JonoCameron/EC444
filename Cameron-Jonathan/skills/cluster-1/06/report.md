#  Quest 1, Skill 06

Author: Jonathan Cameron

Date: 2020-09-07
-----

## Summary
In this exercise, the ESP32 was programmed to have 3 behaviours. Toggle mode, echo mode and decimal to hexadecimal conversion mode.

The behaviours were cycled through by just passing 's' to the board using a switch statement. i.e, mode = 0 would set the 
board to toggle mode, mode = 1, echo mode and mode = 2 enabled dec to hex mode. 

Toggle mode would turn the onboard LED on and off if the board received 't' as a signal. The UART driver was used for this but stdio
was not bypassed. 

Echo mode would simply echo what was passed to the board.

Decimal to hexadecimal mode takes a decimal and returns it as a hexadecimal, using the function atoi().

## Sketches and Photos
Here is a link to it working:
https://youtu.be/f8YHorQ4IHI

## Modules, Tools, Source Used Including Attribution
I used the console I/O brief to take input from the client.

## Supporting Artifacts


-----
