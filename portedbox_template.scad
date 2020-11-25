/* loudspeaker __MODEL__ */
loudspeaker_number = __NUMBER__;
margin = __MARGIN__; /* cm */
loudspeaker_diameter = __DIAMETER__; /* cm */
internal_ported_box_volume = __PORTEDBOXVOLUME__; /* dm³ (Liter) */
ported_box_port_number = __PORTEDBOXPORTNUMBER__;
ported_box_port_slot_activated = __PORTEDBOXPORTSLOTACTIVATED__;
ported_box_port_slot_width = __PORTEDBOXPORTSLOTWIDTH__;
ported_box_port_slot_height = __PORTEDBOXPORTSLOTHEIGHT__;
ported_box_port_diameter = __PORTEDBOXPORTDIAMETER__;
ported_box_port_length = __PORTEDBOXPORTLENGTH__;
wood_thick = __WOODTHICK__; /* cm */
wood_color = "wheat";

internal_ported_box_width = loudspeaker_diameter + margin;  /* cm (x)*/
internal_ported_box_height = max((ported_box_port_diameter + margin) + ((loudspeaker_diameter + margin) * loudspeaker_number), 1.618 * internal_ported_box_width); /* cm (z)*/
internal_ported_box_depth = internal_ported_box_volume * 1000 / (internal_ported_box_width * internal_ported_box_height); /* cm (y)*/

vertical_center = (internal_ported_box_height + (2 * wood_thick)) / 2;
horizontal_center = (internal_ported_box_width + (2 * wood_thick)) / 2;

module speaker_hole(diameter, x, z)
{
    translate([x, 0, z]) {
        rotate([90, 0, 0]) {
            color("black") {
                cylinder($fn=360, h = 2 * wood_thick, r1 = diameter / 2, r2 = diameter / 2, center=true);
            }
        }
    }
}

module port_hole(diameter, slot, width, height, x, z)
{
    if (ported_box_port_slot_activated == true) {
	translate([x, 0, z]) {
	    color("black") {
		cube([width, 2 * wood_thick, height]);
	    }
	}
    } else {
	translate([x, 0, z]) {
	    rotate([90, 0, 0]) {
		color("black") {
		    cylinder($fn=360, h = 2 * wood_thick, r1 = diameter / 2, r2 = diameter / 2, center=true);
		}
	    }
	}
    }
}

module closed_enclosure(width, height, depth)
{
    
    difference() {
	/* external dimentions */
	color(wood_color) {
	    cube([width + (2 * wood_thick), depth + (2 * wood_thick), height + (2 * wood_thick)]);
	}

	/* internal dimentions */
	translate([wood_thick, wood_thick, wood_thick]) {
	    color(wood_color) {
		cube([width, depth, height]);
	    }
	}
    }
}

difference() {
    closed_enclosure(internal_ported_box_width, internal_ported_box_height, internal_ported_box_depth);

    /* calculate ports distribition */
    port_placing = (internal_ported_box_width + (2 * wood_thick)) / (ported_box_port_number + 1);
    for (i = [1:ported_box_port_number]) {
	if (ported_box_port_slot_activated == false) {
	    /* circular ports, center is used */
	    port_hole(ported_box_port_diameter, ported_box_port_slot_activated, ported_box_port_slot_width, ported_box_port_slot_height, i * port_placing, wood_thick + margin + ported_box_port_diameter / 2);
	} else {
	    /* slot ports, left corner is used */
	    port_hole(ported_box_port_diameter, ported_box_port_slot_activated, ported_box_port_slot_width, ported_box_port_slot_height, (i * port_placing) - (ported_box_port_slot_width / 2),wood_thick);
	}
    }

    /* loudpeakers distribution */
    for (i = [0:loudspeaker_number-1]) {
	if (ported_box_port_slot_activated == false) {
		speaker_hole(loudspeaker_diameter, horizontal_center, margin + (loudspeaker_diameter / 2) + (ported_box_port_diameter + margin) + i * (loudspeaker_diameter + margin));
	} else {
		speaker_hole(loudspeaker_diameter, horizontal_center, margin + (loudspeaker_diameter / 2) + ported_box_port_slot_height + i * (loudspeaker_diameter + margin));
	}
    }
}
