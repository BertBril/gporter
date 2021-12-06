# GPORTER GP-102
Setting and reading POIs on a GPORTER GP-102 GPS device for geocaching or other purposes.

## Intention
Sascha bought GPORTER GP-102 device for geocaching. Sadly the included windows software does not allow to set POIs. So the device became pretty useless for geocaching. 
He wrote a simple command line application, that directly can set POIs and read them. So geocaching or google maps coordinates in decimal format can be taken.

Bert just wanted to handle the POIs in his own processing pipeline, and decided to rework Sascha's initial program so he could understand it, and make a small change to the '-r' functionality.


## Compile
gcc gporter_reworked.c -o gporter_reworked

## Install
sudo cp gporter_reworked /usr/local/bin/

## The POI format
Sascha made a simple (but very useful) description in "POI-Format.ods".

## Usage
On Sascha's Linux machine (which has teh automounter active) when the GPORTER GP-102 is connected it will appear in something like "/media/sascha/CANMORE/GP-102".
If you do not use the automounter, try the 'blkid' command. With this info I added and entry to fstab like this:
UUID=4578-49FC /mnt/GPS vfat noauto,users 0 0
After mounting, you'll find the POI's at:
/mnt/GPS/GP-102/POIs
The file names are 000 001 ...

### Setting a POI
Sascha has choosen the parking place of the television tower of Stuttgart. 

https://www.google.de/maps/place/Fernsehturm+Stuttgart/@48.7553172,9.1886679,36m/data=!3m1!1e3!4m5!3m4!1s0x4799c4a78c941ea5:0xee74d8b131b9a572!8m2!3d48.755857!4d9.1901086

Google says it is on:
48.755319, 9.188723

This means N48.755319, E9.188723. 

To set this POI change in the POI directory enter in a shell (bash) "cd /media/username/CANMORE/GP-102/POIs". Enter the following to set the POI 010: 

gporter  -N48.755319 -E9.188723 -o010

If you want to set a symbol, other than the star you can chosse between star, house,  flag, car, eat, bus, gas, skyscraper, plane. 

To set a house symbol on the same coordinate use the -s parameter and enter: 

gporter  -N48.755319 -E9.188723 -o010 -shouse

### Setting a POI with date
The gporter shows in its display POIS in the format xxxx-xxxx. You can set those eight digits by using the -d and -t parameter. If you want to set 5678-1234 enter:

gporter  -N48.755319 -E9.188723 -o010 -shouse -d5678 -t1234

### Reading a POI
To read a coordinate enter enter in a shell (bash) "cd /media/username/CANMORE/GP-102/POIs. Type:

gporter -r 001

This may output a name and coordinate like:

1204-1533 N48.71232 E9.30687

Enter the N/E part one in google maps to see where it is.

## Zest-coordinates 
In the folder gporter_files you'll find some test coordinates.







