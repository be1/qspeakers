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
wood_thick = __WOODTHICK__; /* mm */
wood_color = "wheat";

internal_ported_box_width = loudspeaker_diameter + margin;  /* mm (x)*/
internal_ported_box_golden = max((ported_box_port_diameter + margin) + ((loudspeaker_diameter + margin) * loudspeaker_number), 1.618 * internal_ported_box_width); /* mm (z)*/
internal_ported_box_alt = internal_ported_box_volume * 1000000 / (internal_ported_box_width * internal_ported_box_golden); /* mm (y)*/
internal_ported_box_height = max(internal_ported_box_golden, internal_ported_box_alt);
internal_ported_box_depth = min(internal_ported_box_golden, internal_ported_box_alt);

vertical_center = (internal_ported_box_height + (2 * wood_thick)) / 2;
horizontal_center = (internal_ported_box_width + (2 * wood_thick)) / 2;

module speaker_hole(diameter, x, z)
{
    translate([x, 0, z]) {
        rotate([90, 0, 0]) {
            color("transparent") {
                cylinder($fn=360, h = wood_thick, r1 = diameter / 2, r2 = diameter / 2, center=true);
            }
        }
    }
}

module port_hole(diameter, slot, width, height, x, z)
{
    if (ported_box_port_slot_activated == true) {
        translate([x, 0, z]) {
            color("transparent") {
                cube([width, wood_thick, height]);
            }
        }
    } else {
        translate([x, 0, z]) {
            rotate([90, 0, 0]) {
                color("transparent") {
                    cylinder($fn=360, h = wood_thick, r1 = diameter / 2, r2 = diameter / 2, center=true);
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

    /* calculate ports distribution */
    port_placing = (internal_ported_box_width + (2 * wood_thick)) / (ported_box_port_number + 1);
    for (i = [1:ported_box_port_number]) {
        if (ported_box_port_slot_activated == false) {
            /* circular ports, center is used */
            port_hole(ported_box_port_diameter, ported_box_port_slot_activated, ported_box_port_slot_width, ported_box_port_slot_height, i * port_placing, wood_thick + margin + ported_box_port_diameter / 2);
        } else {
            /* slot ports, left corner is used */
            port_hole(ported_box_port_diameter, ported_box_port_slot_activated, ported_box_port_slot_width, ported_box_port_slot_height, (i * port_placing) - (ported_box_port_slot_width / 2), wood_thick);
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
