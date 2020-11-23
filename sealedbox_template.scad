/* loudspeaker __MODEL__ */
loudspeaker_number = __NUMBER__;
margin = __MARGIN__; /* cm */
loudspeaker_diameter = __DIAMETER__; /* cm */
internal_closed_box_volume = __SEALEDBOXVOLUME__; /* dm³ (Liter) */
wood_thick = __WOODTHICK__; /* cm */
wood_color = "wheat";

internal_closed_box_width = loudspeaker_diameter + margin;  /* cm (x)*/
internal_closed_box_height = max((loudspeaker_diameter + margin) * loudspeaker_number, 1.618 * internal_closed_box_width); /* cm (z)*/
internal_closed_box_depth = internal_closed_box_volume * 1000 / (internal_closed_box_width * internal_closed_box_height); /* cm (y)*/

vertical_center = (internal_closed_box_height + (2 * wood_thick)) / 2;
horizontal_center = (internal_closed_box_width + (2 * wood_thick)) / 2;

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
    closed_enclosure(internal_closed_box_width, internal_closed_box_height,internal_closed_box_depth);
    for (i = [0:loudspeaker_number-1]) {
        speaker_hole(loudspeaker_diameter, horizontal_center, margin + (loudspeaker_diameter / 2) + i * (loudspeaker_diameter + margin));
    }
}
