/* loudspeaker __VENDOR__ __MODEL__ */
loudspeaker_number = __NUMBER__;
margin = __MARGIN__; /* mm */
loudspeaker_diameter = __DIAMETER__; /* mm */
internal_closed_box_volume = __SEALEDBOXVOLUME__; /* dm³ (Liter) */

internal_closed_box_width = loudspeaker_diameter + margin;  /* mm (x)*/
internal_closed_box_golden = max((loudspeaker_diameter + margin) * loudspeaker_number, 1.618 * internal_closed_box_width); /* mm (z)*/
internal_closed_box_alt = internal_closed_box_volume * 1000000 / (internal_closed_box_width * internal_closed_box_golden); /* mm (y)*/
internal_closed_box_height = max(internal_closed_box_golden, internal_closed_box_alt); /* mm (z) */
internal_closed_box_depth = min(internal_closed_box_golden, internal_closed_box_alt); /* mm (y) */

saw_thick = __SAWTHICK__; /* saw cut thick in mm */
wood_thick = __WOODTHICK__;  /* board thick in mm */

vertical_center = (internal_closed_box_height + (2 * wood_thick)) / 2;
horizontal_center = (internal_closed_box_width + (2 * wood_thick)) / 2;

module board(width, height) {
    square(size = [width, height], center = false);
}

module speaker(diam, x, y){
    translate([x, y, 0])
        circle($fn=360, d=diam, true);
}

module board_speaker(width, height, diam){
    difference() {
        board(width, height);
        for (i = [0:loudspeaker_number-1]) {
            speaker(diam, width / 2, margin + (diam / 2) + i * (diam + margin));
        }
    }
}

/* boards */
front_width = internal_closed_box_width + (2 * wood_thick); /* mm */
front_height = internal_closed_box_height + (2 * wood_thick); /* mm */
side_depth = internal_closed_box_depth; /* mm */
side_height = internal_closed_box_height; /* mm */
top_width = front_width; /* mm */
top_depth = side_depth; /* mm */

/* two times */
for (i = [0:1]) {
    translate([0, i*(max(front_height,front_width,side_depth,side_height) + saw_thick), 0]) {
        /* one board with speaker */
        translate([0, 0, 0]) 
            board_speaker(front_width, front_height, loudspeaker_diameter);
        
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
