# hydroponics
A very DIY hydroponic system

FYI: The code is messy and I'll try and clean it up a bit in future commits. 

This work was entirely inspired by [Eleooo]( http://eliooo.com/) and because I'm a huge nerd, I decided to apply microcontrollers. 

The system I ended up building is a hybrid "continuous flow" and "ebb/flow" system - the solution circulates for 15 min every hour, refreshing the solution in the "standard" grow tanks, and "watering" the ebb/flow tank. Because of how the standard tanks are connected together, the pump can't run continuously or they'll overflow, hence the arduino control which runs the pump 20 seconds of every minute for 15 minutes. (Pictures in the diagram folder to come). 

I opted for this model to support growing root veggies like carrots. 

## Construction
### Overview
You have a reservoir which will hold the nutrient solution. A pump pulls the solution and delivers it to the top most container. Solution flows through the containers and drains back into the reservoir.

When flowing horizontally, be aware of height, you'll want to slightly elevate the "upstream" container or else the solution won't flow properly. 
### Materials
* Plastic containers (ideally opaque to prevent algae growth) which will be the planters, and a larger one for the reservoir
* Lids for the containers
* Vinyl tubing
* Hose connectors (see 3d files)
* Aeration pump
* Fluid pump (immersible) x2 (one to purge system)
* Nutrient solution (I've been using General Hydroponics Flora Series)
* Hydroponic Expanded clay pebbles or other substrate of your choice
* Hydroponic baskets (to hold the pebbles and plants)
* LED Grow lights (I chose yellow/white color ones for aesthetics)
* Marine adhesive for waterproofing and securing the connectors

#### Control system materials
* Arduino MKR Wifi 1010 (you can also use a ESP8266). Network connectivity is recommended for NTP and telemetry
* 4 channel relay board with opto-isolator drivers (if you're using a 3.3v microcontroller you'll use separate trigger (3.3v) and coil (5v) voltages. You can pull the 5v supply from the Arduino's pin if you're powering it via a USB supply.
* 4 120v outlets (or a power-strip you can modify)
* PH meter for PH feature
* PH control system (TODO)


### Steps

#### Overview
1. Print connectors (optional)
1. Drill or melt holes in containers (for the hose connectors) and lids (for the plant baskets). You'll want about 6-8cm clearance between baskets, depending on basket size and the plants you intend to grow.
1. Insert baskets into holes
1. Put a bead of adhesive on flange, insert connector, bead the screw-on flange, and screw on (screw goes on outside of container)
1. Arrange containers as needed
1. For containers that drain vertically (down), you'll want to cut a bit of tubing to put on the inside of the container to ensure the fluid level is just touching the bottom of the baskets
1. Hook up the tubing
1. Set up lights and plug everything in (make sure that each device - pump, lights, air pump - are plugged into their appropriate outlet) 
1. Place pump in reservoir, connect, fill reservoir with water and test. Depending on your setup, you'll want to adjust the pump runtime per cycle (`PUMP_PULSE_LENGTH`) in `src/constants_and_pins.h` 
1. Transplant plants into baskets, covering with washed clay pebbles. 
1. Follow nutrient manufacturer's guides

#### Control system
1. Select 4 appropriate digital I/O pins for the relay board and connect 
1. Make sure if using a 3.3 v board, the jumper that bridges signal and coil voltages on the relay control board is removed. You'll supply 5v separately. 
1. Connect ground
1. Connect 5v to relay coil input VCC
1. Load up Arduino IDE and Modify `constants_and_pins.h` with the pins chosen 
1. Flash and test. 
1. Wire up outlets to the relay board. I recommend testing each outlet against the controller with a continuity tester before supplying voltage


## Repository Structure
### 3d files
This dir contains an OpenSCAD file which can generate STL files for the various types of hose connectors used. There is a function which will also generate bulkhead connectors with a threaded washer. 

3d printing may be much less expensive for you, as hardware stores sell the connectors for about $5-8 each!

You'll want to experiment with your print settings to achieve a water-tight print. I found with my Ender3, a 150% extrusion at .16 layer-height, solid infill was decent. You'll likely have to do some tests.

The two functions you probably care about are 
> print_connector_part(num=2, flanged=true);

and

>print_flange_screw();

Specify the diameter of your tubing (OD) , and print away! The default OD in the OpenSCAD file is 16.25mm.



### src
This directory contains the Arduino source for the control system. This was implemented on a MKR WIFI1010 and relies on Wifi and NTP to work correctly. HOWEVER, you can use a much simpler device as long as it has a reasonably reliable clock. You'll just need to add code and a user interface for setting the time. 

All output pins are connected to a relay control board (mine's a 5v coil, 3v trigger) which then switch the 120v pumps and lighting. The pH pumps are 12v but also need relays. If you've got a 6 or 8-channel relay board, you can use the one board, just be careful about mixing up the 12 VDC and 120 VAC ;) 

### diagrams
TBD. This directory will contain wiring and other diagrams describing the physical layout of the system. 