
width = 80;
distanceBetweenHoles = 72; //
fanDuctDiameter = 74;
$fn = 128;

holeOffset = (width-distanceBetweenHoles) / 2;

module basePlate() {
    difference() {
        union() {
            cube([width,width,2]);
        }
        union() {
            translate([width/2, width/2,0]) {
                cylinder(d=fanDuctDiameter, h=2);
            }
        }
    }
}

module fanScrewHoles() {
    translate([holeOffset,holeOffset,0]) {
        cylinder(d=4, h=3);
    }

    translate([width -holeOffset,holeOffset,0]) {
        cylinder(d=4, h=3);
    }

    translate([holeOffset,width-holeOffset,0]) {
        cylinder(d=4, h=3);
    }

    translate([width -holeOffset, width - holeOffset,0]) {
        cylinder(d=4, h=3);
    }
}

ductHeight = 100;
ductDepth = 50;

module hollowCube() {
    translate([0,0,5]) {
        rotate([12,0,0]) {
            difference() {
                union() {
                    translate([(width-fanDuctDiameter+8)/2, 6, 0]) {
                        cube([fanDuctDiameter-8, ductHeight, ductDepth]);
                    }
                }
                union() {
                    translate([(width-fanDuctDiameter+8)/2+4, 6+4, 0]) {
                        cube([fanDuctDiameter-8-8, ductHeight+8, ductDepth-2]);
                    }
                }
            }
        }
    }
}


difference() {
    union() {
        basePlate();
        // Hull section between basePlate and hollow cube.
        hollowCube();
    }
    union() {
        fanScrewHoles();
    }
}