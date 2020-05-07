
// Module loads
include <threads.scad>;

overlap = 0.001;

OD = 16.25;
OR = OD/2;

ID = 10;
IR = ID/2;
flange_height = 3;
conetop = IR+((OR-IR)/2);

length = 18;
sections = 5;
section_length = length/(sections);
//standoff = section_length;
standoff=10;

/* 
Select how many connector ends by setting 'num' to an integer between 1 and 5 for best results
If you want a flange on the connector, use flanged=true
*/
print_connector_part(num=2, flanged=true);

//print_flange_screw();
// Helper Print Statements
//print_cone_section();
//print_cone_sections();
//print_single_connector($fn=24);
//print_flanged_connector(3, $fn=24);

//print_connectors(2, $fn=24);
//print_connectors(3, $fn=24);
//print_connectors(4, $fn=24);
//print_connectors(5, $fn=24);
//print_connectors(6, $fn=24);
//print_connectors(8, $fn=24);

//print_elbow($fn=24);
//print_elbow(angle=45, $fn=24);
//print_elbow(angle=120, $fn=24);

module print_connector_part(num=1, flanged=false)
{
    if (flanged==true) {
        print_flanged_connectors(num, $fn=24);
    } else {
        if (num == 1) {
            print_single_connector($fn=24);
        } else {
            print_connectors(num, $fn=24);
        }
    }
}

module print_cone_section()
{
	conebottom = OR;

	difference()
	{
		cylinder(r1=conebottom, r2=conetop, h=section_length+overlap);
	
		translate([0,0,-overlap])
		cylinder(r=IR, h=section_length+overlap*3);
	}
}

module print_cone_sections()
{
	for (section = [0:sections-1])
	{
		translate([0,0,section*section_length])
		print_cone_section();
	}
}

module connector_blank()
{
	cylinder(r=IR, h=standoff+overlap*3);
	sphere(r=IR);
}
module print_flanged_connectors(num)
{
    if (num==1) {
        print_single_connector();
    } else {
        print_connectors(num);
    }
    
    if (num>2) {
        translate([0,0,standoff])
            if (num <= 4) 
                flange(IR);
    } else {
      flange(IR);
    }
}

module print_flange_screw()
{
    difference(){
        cylinder(r=IR*4, h=flange_height*2, center=true);
       translate([0,0,-flange_height*2+1])
        metric_thread(OD,1,flange_height*3, internal=true);
    }
    
}


module print_single_connector()
{
	translate([0,0,standoff-overlap])
	print_cone_sections();

	difference()
	{
		cylinder(r=conetop, h=standoff);
        
		translate([0,0,-overlap])
       
		connector_blank();
       
	}
    metric_thread(OD,1,standoff);
    
}
module flange(ir)
{
    difference()
    {
        cylinder(r=ir*4, h=flange_height, center=true);
        cylinder(r=ir, h=6, center=true);
        
        }
    
    

      
}

module print_connectors(num)
{
	angle = 360/num;

	difference()
	{
		union()
		{
			sphere(r=conetop);
			for(connector = [1:num])
			{
				rotate([(connector-1)*angle, 0, 0])
				print_single_connector();
			}
		}

		for(connector = [1:num])
		{
			rotate([(connector-1)*angle, 0, 0])
			connector_blank();
		}
	}

}

module print_elbow(angle=90)
{	
	difference()
	{
		union()
		{
			sphere(r=conetop);

			print_single_connector();
			rotate([angle, 0, 0])
			print_single_connector();
		}

		// subtract out the rounded bottom
		// for each connector
		connector_blank();

		rotate([angle, 0, 0])
		connector_blank();
	}
}
