# hydroponics
A very DIY hydroponic system

FYI: The code is messy and I'll try and clean it up a bit in future commits. 

This work was entirely inspired by [Eleooo]( http://eliooo.com/) and because I'm a huge nerd, I decided to apply microcontrollers. 

The system I ended up building is a hybrid "continuous flow" and "ebb/flow" system - the solution circulates for 15 min every hour, refreshing the solution in the "standard" grow tanks, and "watering" the ebb/flow tank. Because of how the standard tanks are connected together, the pump can't run continuously or they'll overflow, hence the arduino control which runs the pump 20 seconds of every minute for 15 minutes. (Pictures in the diagram folder to come). 

I opted for this model to support growing root veggies like carrots. 


## Repository Structure
### 3d files
This dir contains an OpenSCAD file which can generate STL files for the various types of hose connectors used. There is a function which will also generate bulkhead connectors with a thrededed washer. 

It will also contain STL files for e.g. the housings for the arduino. 

### src
This directory contains the Arduino source for the control system. This was implemented on a MKR WIFI1010 and relies on Wifi and NTP to work correctly. HOWEVER, you can use a much simpler device as long as it has a reasonably reliable clock. You'll just need to add code and a user interface for setting the time. 

All output pins are connected to a relay control board (mine's a 5v coil, 3v trigger) which then switch the 120v pumps and lighting. The pH pumps are 12v but also need relays. If you've got a 6 or 8-channel relay board, you can use the one board, just be careful about mixing up the 12 VDC and 120 VAC ;) 

### diagrams
TBD. This directory will contain wiring and other diagrams describing the physical layout of the system. 