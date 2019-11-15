include <bat.scad>
include <disp.scad>
include <enc.scad>
include <pcb.scad>

$fa = 0.2;
$fs = 0.2;

stand_d = 2;

pcb_x = 4.65;
pcb_z = 30.8-pcb_h;

// rotate([90, 0, 0]) bat();

// Circuit
union() {
    rotate([90, 0, 0]) disp();
    translate([pcb_x, disp_head_d+pcb_head_d, pcb_h+pcb_z]) 
        rotate([-90, 0, 0]) color("purple") pcb();
}

// Display legs
union() {
    l_w = 5;
    r_w = 5;
    olap = 8;
    pad = 0.2;
    // stands
    translate([-stand_d, -disp_d-disp_screen_d, -stand_d])
        cube([l_w+stand_d, stand_d+disp_d+disp_screen_d, stand_d]);
    translate([-stand_d, pad, 0]) cube([stand_d+l_w, stand_d-pad, olap]);
    translate([-stand_d, -disp_d-disp_screen_d, 0]) cube([stand_d+l_w, disp_screen_d-pad, olap]);
    translate([-stand_d, -disp_d-disp_screen_d, 0]) cube([stand_d-pad, stand_d+disp_d+disp_screen_d, olap]);

    difference() {
        translate([disp_w-r_w, -disp_d-disp_screen_d, -stand_d])
            cube([r_w+stand_d, stand_d+disp_d+disp_screen_d, olap]);
        translate([disp_w-r_w-0.1, -disp_d-pad, 0])
            cube([r_w+pad+0.1, disp_d+2*pad, olap]);
    }

    
}

translate([pcb_x, disp_head_d+pcb_head_d, pcb_z-stand_d])
    cube([pcb_w, pcb_d, stand_d]);
