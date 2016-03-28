
// Max height 270mm.
// Make 200mm height so 70mm bottom of box for raised
// area that will allow solvent to drain from part.
MaxHeight = 250;
MaxHeight = 70;
//MaxHeight = 35;

blockSize = 35; // 35mm (excludes key slot).

blockKeyWidth = 6;

blockDepth = 6;

// 1 == Single
// 2 == Double with middle strip.
blockType = 2;

$fn=50;

module ledGrip() {
    
                
    // LED strip holes
    translate([0,0,0.5]) {
        cube([10,10,3]);
    }

    // 2mm offset either side to keep hold of the LED.
    translate([2,0,0]) {
        cube([6,10,0.5]);
    }
}

// If multiple blocks use zero tollerance
// includeMiddleStripHolder - if holder for LED strip in the middle of the two main ones.
// i.e. for white LED strip between the two UV strips.
module block(blockCount, blockConnectTollerance, includeMiddleStripHolder) {

length = (blockCount * blockSize) + 5 ;

    difference() {
        union() {
            translate([0,2,0]) {
                cube ([length,blockKeyWidth,blockDepth]);
            }
            
            // Move down 10mm
            // Add outer blocks either side
            // to connect one segment to the next.
            translate([5,8,0]) {
                cube([length,2,blockDepth]);
            }
            
            translate([5,0,0]) {
                cube([length,2,blockDepth]);
            }
        }
        union() {
            
            for (offset = [16 : 35 : length]) {
            
                // Mounting holes
                translate([offset,5,0]) {
                    cylinder(d=3.5,h=blockDepth);
                    
                    // Large hole for head of screw
                    translate([0,0,3]) { 
                        cylinder(d=6,h=blockDepth-3);
                    }
                    
                }
            }
            
            // Holes to hold the LED strip in place.
            for (offset = [20 : 35 : length-10]) {
                translate([offset,0,0.0]) {
                    ledGrip();
                }
            }
            
            // for double strips, insert a middle
            //  
            if (includeMiddleStripHolder) {
                for (offset = [37.5 : 70 : length]) {
                    translate([offset,0,0.0]) {
                        ledGrip();
                    }
                }
            }
            
            // cut out a tiny bit extra from the open end to 
            // allow for variance in printing
            // Offset is 2mm either wide.
            translate([length,2 - blockConnectTollerance,0]) {
                // Allow 0.5mm spacing for key to fit.
                cube ([5,blockKeyWidth + (2* blockConnectTollerance),blockDepth]);
            }
        }
    }
}

//block(2, 0.5, true);
//block(2, 0.5, false);

if (blockType == 1) {
    block(1, 0.5, false);
} else if (blockType == 2) {
    block(2, 0.5, true);
}

