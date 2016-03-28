Form Keep
=========

A UV exposure box and Tray store (to keep your trays in) for the Form 1, 1+ and 2 printers.

![Form Keep UV box and tray storage][MainPicture]


Features:
* UV exposure in base unit on 30 minute timer with countdown "clock" style neopixel display on front door.
* 3x shelfs for tray storage.
* 1x top open shelf for sand paper storage
* 1x small (25mm) area under lowest shelf for possible other storage
* Door sensors for top unit to tell if the door is open, and hence light getting to trays
* Ventilation in the base unit to prevent a build up of IPA vapours in the box
* White LEDs in base unit to allow the part to be seen clearly when the door is open


Construction:
* Top and bottom units are made of 12mm MDF (2 sheets 600x1200mm)
* Door on base made from 18mm MDF
* Feet made from 12mm MDF glued to base, then adjustable feet set into that
* Door for top unit made with 3mm acrylic
* Shelfs made with 5mm acrylic, colour to match the resin
* Front door panel 3mm clear acrylic
** 3D printed support for 12 pixel NeoPixel ring for count down timer
* Fan panel made with 3mm acrylic (ideally a scrap piece as it's not seen)
* Fan is 80mm PC fan (with yellow pulse wire ignored)
* Base panel 3mm acrylic. Can be any color
* Inside is coated with tin foil (glued to the MDF) to help reflect the UV rays

Cut using CNC router, can be hand cut if no CNC available using the svg files as cutting plans.

Roundovers done by hand router as it's quicker. 
* Bottom of the base unit has large round over on each side coming out thought the door but not along door.
* Top of base unit has small round over on sides and front of door.

Painting:
* Base unit grey (The grey in the picture is actually just undercoat which works well).
* Top unit orange

Electronics:
* Uses V2.00 Kitchen Lights Controller from https://github.com/Tinamous/KitchenLightsController
* Channel 1 & 2 used to drive UV LEDs. 
* 2x 5M strips of UV LEDs, split into 900mm lengths (to make it easier to fit), each individually wired back to the PCB.
* 2x 300mm White LEDs connected to channel 3
* 1x PC fan connected to channel 4 with protection diode across either the terminal block or headers (depending on which is free).
* USB power controller not fitted to board.
* Max 0.05R current sense resistor (5-10A load due to UV LEDs).
* Hall effect transistor used for door sensor (rather than have switch in possible explosive environment of IPA vapours).
** Connected to Button input on PCB
* Hall effect sensor to top unit door on PIR input
* TEMT6000 breakout board for light level sensor on top unit connected to Light Sensor on PCB
* Optional temperature sensors in base and top units connected to temperature sensor input
* Optional buzzer wired on breakout area of PCB.
* 12 pixel NeoPixel ring. 12th (last) pixel should be at 12 O'Clock position
** https://www.adafruit.com/products/1643
* Use 5V switching regulator rather than 7805 due to NeoPixel load (see eBay), or fit good heatsink to 7805. (360mA max load from NeoPixels, + 100mA from Photon).
* Needs a good 5-10A 12V DC power supply.
* Use Particle Photon to drive, connect to Tinamous.com to allow remote monitoring of light, temperature and more inportantly time remaining.




Bugs
----

* Back Panel slots are out of alignment with side panel slots on top unit.
* Top unit slots are to shallow (3mm, with 1mm error on Z alignment due to MDF varience and 1mm allowance for tollerance on the acrylic shelfs this leaves a potential 1mm overlap on one side.
* Front foor, opening is far to big (much bigger than Form2 display).
* Ribon cable pocket is not deep enough on base.
* Top shelf currently split in 2 which is not ideal for stopping the light get into the top resin tray. Needs an extra sheet.

Timing: See http://formlabs.com/support/finishing/post-curing-prints/


[MainPicture]: https://github.com/Tinamous/FormKeep/raw/master/Images/FormKeep-2016-03-25-InProgress.png "Form Keep UV box and tray storage - In Progress"