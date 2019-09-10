# WS2812B LED Strip Controller

Simple program meant to be run on an ESP8266 connected to a strip of WS2812B leds. WS2812Bs are individually addressable leds, so utilizing control signals from an arduino we can have fancy patterns and automate through a simple serial terminal on the computer plugged into the arduino.

Control through the terminal by typing parameter=value, where possible params are 'm', for mode, 'r', for red value, 'g', for green value, 'b', for blue value, 'f', for first, 'l', for last, and 'i', for index. After hitting enter, the program will attempt to parse your input, and repeat back the values it now has stored for the parameters. When you're happy with these values, simply sending z to the program will cause it to update its operation based on the values.

Mode has a few options, 0 will reset any stored color data, 1 will set every ith led between f and l to the rgb vals, 2 will store the given rgb vals in a color data array for blending and patterns, 3 and 4 will configure first/last/index for color data modes, as well as 3 triggers blending, while 4 triggers patterns, and 5 initializes some basic data in the color data array. RGB are limited from 0-255, as these are the possible values for the way the color is built, and FLI are limited from 0 to the number of leds you have marked.

We have a long color data array that basically stores 3 ints at each index to hold a single particular state of rgb. With this, we can have blending occur between each state over a period of time, or have patterns that 'move along' the strip.

Example procedures:

m=1 r=255 g=0 b=0 f=0 l=150 i=1 z : will set every led from 0 to 150 to max red value, and 0 blue/green value.
m=1 g=255 i=2 z                   : will set every second led from stored first to stored last to maximum green value, as well as whatever red/blue is currently stored
m=2 r=255 g=255 b=255 z           : will store current state of color data array as r=255, g=255, b=255, and increment the state counter
m=3 f=20  z                       : will set blending/patterns to start occuring at 20th led, as well as trigger blending
m=4 l=30  z                       : will limit blending/patterns to the 30th led, as well as trigger patterns
m=5 z                             : will initialize the first three color states in our array to be full red, full blue, full green, and change the state number appropriately
b=255                             : will change blue parameter to full 255
z                                 : will cause current parameters to be handled by our control method

Note that removing z from these options will cause visible change to the parameters, but the change will not have any effect until z is ran and the parameters are handled.

Requires FastLED.h, which I haven't directly included, but you can find at http://fastled.io/.

Customizing this to your specific setup should be fairly straight forward; what mostly matters is that you change the definitions at the top of main.cpp for NUM_LEDS and DATA_PIN if your strip does not have 150 leds, and if you don't use the 4th data pin as your connection to the data connection of the WS2812Bs. In theory this works for other boards or LED strips if you change the specs in platformio.ini, and the baud rate/strip type in setup(), but as this is mostly for my personal use I haven't checked.

Most of the committed files are vscode boilerplate, actual program lives in /src/main.cpp, but you should be able to copy it into your basic arduino ide and flash it. However, I haven't tried since I moved to platformio on vscode, and I recommend you use it over the default arduino ide.

Note: I originally built this with wifi in mind, but scrapped most of the code I had written as I moved back to college and didn't feel like tangling with my network, even though the point was originally to abstract the LED controller behind a nicer interface that could then control other 'home automation' devices simultaneously. I'll probably bring the wifi code back in at some point, but since I can't test it and this is really just quick and dirty for personal use, I won't commit it right now. Regardless, even if these chips have built in wifi, they're also like six bucks on amazon, so I think they're good to use anyway.