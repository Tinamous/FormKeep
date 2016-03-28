@echo off

@echo Building Fan Duct
"C:\Program Files\OpenSCAD\openscad.com" -o FanDuct.stl FanDuct.scad

@echo Building Front Panel Support (Normal)
"C:\Program Files\OpenSCAD\openscad.com" -o FrontPanelSupport.stl -D Height=48 FrontPanelSupport.scad

@echo Building Front Panel Support (High)
"C:\Program Files\OpenSCAD\openscad.com" -o FrontPanelSupport-High.stl -D Height=65 FrontPanelSupport.scad

@echo Building PCB Standoffs
"C:\Program Files\OpenSCAD\openscad.com" -o PcbStandoffs.stl -D Height=65 PcbStandoffs.scad

@echo Building Single LED strip holder
"C:\Program Files\OpenSCAD\openscad.com" -o LedStripHolder-Single.stl -D blockType=1 LedStripHolder.scad

@echo Building Double with Middle LED strip holder
"C:\Program Files\OpenSCAD\openscad.com" -o LedStripHolder-DoubleWidth-TrippleHolder.stl -D blockType=2 LedStripHolder.scad





