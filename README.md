# LeoGeo App
A desktop application which interfaces with a custom built prototype 'Reverse Geocaching' device, allowing you to set target coordinates, download log data, view data, unlock the device, etc.

## Building
for an optimised build, with no debug symbols, simply open a terminal in the project directory and type:
```
make build-release
```

for an unoptimised build, with debug symbols:
```
make setup
make build
```

###Linux:
the executable will be output as:
```
./build/bin/{Debug/Release}/LeoGeo
```
###Mac:
it will be output as an app package, as:
```
./build/bin/{Debug/Release}/LeoGeo.app
```
and the executable itself can be found at:
```
./build/bin/{Debug/Release}/LeoGeo.app/Contents/MacOS/LeoGeo
```
