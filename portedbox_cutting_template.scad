/* loudspeaker __VENDOR__ __MODEL__ */
loudspeaker_number = __NUMBER__;
margin = __MARGIN__; /* mm */
loudspeaker_diameter = __DIAMETER__; /* mm */
internal_ported_box_volume = __PORTEDBOXVOLUME__; /* dm³ (Liter) */
ported_box_port_number = __PORTEDBOXPORTNUMBER__;
ported_box_port_slot_activated = __PORTEDBOXPORTSLOTACTIVATED__;
ported_box_port_slot_width = __PORTEDBOXPORTSLOTWIDTH__; /* mm */
ported_box_port_slot_height = __PORTEDBOXPORTSLOTHEIGHT__; /* mm */
ported_box_port_diameter = __PORTEDBOXPORTDIAMETER__; /* mm */
ported_box_port_length = __PORTEDBOXPORTLENGTH__; /* mm */

internal_ported_box_width = loudspeaker_diameter + margin;  /* mm (x)*/
internal_ported_box_golden = max((ported_box_port_diameter + margin) + ((loudspeaker_diameter + margin) * loudspeaker_number), 1.618 * internal_ported_box_width); /* mm (z)*/
internal_ported_box_alt = internal_ported_box_volume * 1000000 / (internal_ported_box_width * internal_ported_box_golden); /* mm (y)*/
internal_ported_box_height = max(internal_ported_box_golden, internal_ported_box_alt);
internal_ported_box_depth = min(internal_ported_box_golden, internal_ported_box_alt);

saw_thick = __SAWTHICK__; /* saw cut thick in mm */
wood_thick = __WOODTHICK__;  /* board thick in mm */

vertical_center = (internal_ported_box_height + (2 * wood_thick)) / 2;
horizontal_center = (internal_ported_box_width + (2 * wood_thick)) / 2;

module board(width, height) {
    square(size = [width, height], center = false);
}

module speaker(diam, x, y){
    translate([x, y, 0])
        circle($fn=360, d=diam, true);
}

module port(diam, slot, width, height, x, y)
{
    if (slot == true) {
	translate([x, y, 0]) {
	    square(size = [width, height], center = false);
	}
    } else {
	translate([x, y, 0]) {
	    circle($fn=360, d=diam, true);
	}
    }
}

module board_speaker_port(width, height, diam){
    difference() {
	board(width, height);

	/* calculate ports distribition */
	port_placing = (internal_ported_box_width + (2 * wood_thick)) / (ported_box_port_number + 1);
	for (i = [1:ported_box_port_number]) {
	    if (ported_box_port_slot_activated == false) {
		/* circular ports, center is used */
		port(ported_box_port_diameter, ported_box_port_slot_activated, ported_box_port_slot_width, ported_box_port_slot_height, i * port_placing, wood_thick + margin + ported_box_port_diameter / 2);
	    } else {
		/* slot ports, left corner is used */
		port(ported_box_port_diameter, ported_box_port_slot_activated, ported_box_port_slot_width, ported_box_port_slot_height, (i * port_placing) - (ported_box_port_slot_width / 2), wood_thick);
	    }
	}

        for (i = [0:loudspeaker_number-1]) {
	    if (ported_box_port_slot_activated == false) {
		speaker(diam, width / 2, margin + (diam / 2) + (ported_box_port_diameter + margin) + i * (diam + margin));
            } else {
                speaker(diam, width / 2, margin + (diam / 2) + ported_box_port_slot_height + i * (diam + margin));
            }
        }
    }
}

/* boards */
front_width = internal_ported_box_width + (2 * wood_thick); /* mm */
front_height = internal_ported_box_height + (2 * wood_thick); /* mm */
side_depth = internal_ported_box_depth; /* mm */
side_height = internal_ported_box_height; /* mm */
top_width = front_width; /* mm */
top_depth = side_depth; /* mm */

/* two times */
for (i = [0:1]) {
    translate([0, i*(max(front_height,front_width,side_depth,side_height) + saw_thick), 0]) {
        /* one board with speaker */
        translate([0, 0, 0]) 
	    board_speaker_port(front_width, front_height, loudspeaker_diameter);
        
        /* one back board */
        translate([1 * (front_width + saw_thick), 0, 0])
            board(front_width, front_height);
        
        /* two side boards */
	for (j = [0:1]) {
	    translate([2 * (front_width + saw_thick) + j * (side_depth + saw_thick), 0, 0])
                board(side_depth, side_height);
        }
        
        /* two top/bottom boards */
	for (j = [0:1]) {
	    translate([j * (top_width + saw_thick) + (2 * (side_depth + saw_thick)) + (2 * (front_width + saw_thick)), 0, 0])
                board(top_width, top_depth);
        }
    }
}
