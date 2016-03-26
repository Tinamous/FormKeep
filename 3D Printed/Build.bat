@echo off
REM Requires openscad folder in path.

@echo Building Single
REM Single
"C:\Program Files\OpenSCAD\openscad.com" -o LedStripHolder-Single.stl -D blockType=1 LedStripHolder.scad

@echo Building Double with Middle Stripd
"C:\Program Files\OpenSCAD\openscad.com" -o LedStripHolder-DoubleWidth-TrippleHolder.stl -D blockType=2 LedStripHolder.scad

@echo Building Front Panel Support
"C:\Program Files\OpenSCAD\openscad.com" -o FrontPanelSupport.stl -D Height=48 FrontPanelSupport.scad
"C:\Program Files\OpenSCAD\openscad.com" -o FrontPanelSupport-High.stl -D Height=65 FrontPanelSupport.scad