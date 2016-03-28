$fn = 200;

// Ideal panel width was 115mm
// but cut was outside marking
// so is actually 143mm wide with 9mm overlap.
// If you cut yours correctly reduce this.
// Excludes possible fixing to wooden door.
PanelWidth = 143; // mm Actylic panel only. 

// Offset from left/right side for the mounting holes on the panel
MountingHolesOffset = 5;

// Height of the Acrylic panel
// need to exclude top 12-15mm for unit overlap. 
// Botton has rounded corners.
PanelHeight = 151; // mm

NeoPixelRingOuterDiameter = 38; // mm
NeoPixelRingInnerDiameter = 23; // mm

Height = NeoPixelRingOuterDiameter + 10 + 17; // 5mm top/bottom.
Depth = 8; // mm

NeoPixelXPosition = 110; 
NeoPixelYPosition = Height / 2; //24mm from the top. // Height / 2;
NeoPixelHeight = 4.5; // mm - includes wires (& probably hotmelt)

module frontPanelInsert() {
    difference() {
        union() {
            cube([PanelWidth, Height, Depth]);
        }
        union() {
            // Mounting holes
            translate([MountingHolesOffset, Height/2, 0]) {
                cylinder(d=4, h=Depth, centre=true);
                cylinder(d=8, h=3, centre=true);
            }
            translate([PanelWidth-MountingHolesOffset, Height/2, 0]) {
                cylinder(d=4, h=Depth, centre=true);
                cylinder(d=8, h=3, centre=true);
            }
           
            
            // Hole through the block for light to come through the center of the NeoPixel Display
            translate([NeoPixelXPosition, NeoPixelYPosition, 0]) {
                cylinder(d=NeoPixelRingInnerDiameter, h=Depth, centre=true);
                
                translate([0,0, Depth-NeoPixelHeight]) {
                    cylinder(d=NeoPixelRingOuterDiameter, h=NeoPixelHeight, centre=true);
                }
            }
            
            // Pocket for NeoPixel ring to sit in.
            
            // Channels for wires to escape from NeoPixels.
            // 2mm from top, exit to the right of the neo pixels
            translate([NeoPixelXPosition, NeoPixelYPosition + (NeoPixelRingOuterDiameter/2)-2-3, 2]) { 
                // Din/DOut wires.
                cube([PanelWidth - NeoPixelXPosition, 2, Depth-2]);
            }
            
            translate([NeoPixelXPosition, NeoPixelYPosition - (NeoPixelRingOuterDiameter/2), 2]) { 
                // +Ve and GND
                cube([PanelWidth - NeoPixelXPosition, 2, Depth-2]);
            }
            
            // Oval cutout to look like Form1.
            translate([30, NeoPixelYPosition, 0]) {
                cylinder(d=NeoPixelRingOuterDiameter, h=Depth, centre=true);
                
                translate([0,-(NeoPixelRingOuterDiameter/2), 0]) {
                    cube([35 ,NeoPixelRingOuterDiameter,Depth]);
                }
                
                translate([35,0, 0]) {
                    cylinder(d=NeoPixelRingOuterDiameter, h=Depth, centre=true);
                }
            }
            
            // cut out for Hall Effect door sensor.
            magnetHeightAllowance = 2; // mm
            translate([PanelWidth-10, Height-magnetHeightAllowance, 0]) {
                // Cut out all the way trough to allow for the magnet offset from 
                // the lid of the unit.
                #cube([10, magnetHeightAllowance, 10]);
                
                // Make a pocket for the hall effect device.
                // Move in slightly to allow for x-axis error on magnet alignment.
                // Move down slightly to allow for a wall on the panel
                // Move up down so as to not cut all the way through the panel.
                // and mount it at the back of the panel
                translate([2, -3, 2]) {
                    // Hall effect is about 1.6mm deep
                    #cube([10, 1.5, Depth-2]);
                }
            }
            
            // DEGUB!
            // Chop out most of the panel to get just the hall effect section
            // in order to test print
            cube([PanelWidth-15, Height, Depth]);
        }
    }
}

frontPanelInsert();

// For SVG explort

projection( cut=false ) {
//    frontPanelInsert();
}
