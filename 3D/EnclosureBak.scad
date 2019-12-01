include <bat.scad>
include <disp.scad>
include <enc.scad>
include <pcb.scad>

$fa = 0.2;
$fs = 0.2;

stand_d = 2;
clearance = 0.5;

pcb_x = 4.65;
pcb_z = 30.8-pcb_h;

// innards
translate([0, 7.28, 0]) union() {
    translate([0, 55, 0]) rotate([90, 0, 0]) bat();
    
    translate([2, 0, 20]) rotate([-25, 0, 0])
    union() {
        components();
        disp_case();
        pcb_legs();
    }
}


//components();

module components() { union() {
    rotate([90, 0, 0]) disp();
    translate([pcb_x, disp_head_d+pcb_head_d, pcb_h+pcb_z]) 
        rotate([-90, 0, 0]) color("purple") pcb();
}}

module disp_case() { union() {
    s = 1;
    l_w = 5;
    r_w = 5;
    h = 10;
    clear = 0.2;
    // stands
    difference() {
        translate([-stand_d, -disp_d-disp_screen_d-s, -stand_d])
            cube([l_w+stand_d, stand_d+disp_d+disp_screen_d+s, h]);
        translate([-clear, -disp_d-clear, 0])
            cube([l_w+stand_d, disp_d+2*clear, h]);
    }

    difference() {
        translate([disp_w-r_w, -disp_d-disp_screen_d-s, -stand_d])
            cube([r_w+stand_d, stand_d+disp_d+disp_screen_d+s, h]);
        translate([disp_w-r_w-0.1, -disp_d-clear, 0])
            cube([r_w+clear+0.1, disp_d+2*clear, h]);
    }
    // screen
    difference() {
        translate([-stand_d, -s-disp_d-disp_screen_d-clearance, -stand_d-4.5])
            cube([disp_w+2*stand_d, s, disp_h+2*stand_d+4]);
        translate([disp_view_beg_x, -s-disp_d-disp_screen_d-clearance-0.1, disp_view_beg_y])
            cube([disp_view_w, 2, disp_view_h]);
    }
}}

// PCB legs
module pcb_legs() { union() {
    l_w = 12;
    l_h = 4;
    r_w = 7;
    r_h = 7;
    clear = 0.2;
    difference() {
        translate([pcb_x-stand_d, disp_head_d+pcb_head_d-stand_d, pcb_z-stand_d])
            cube([l_w, pcb_d+2*stand_d, l_h]);
        translate([pcb_x-clear, disp_head_d+pcb_head_d-clear, pcb_z])
            cube([l_w, pcb_d+2*clear, l_h]);
    }
    difference() {
        translate([pcb_x+pcb_w-r_w+stand_d, disp_head_d+pcb_head_d-stand_d, pcb_z-stand_d])
            cube([r_w, pcb_d+2*stand_d, r_h]);
        translate([pcb_x+pcb_w-r_w+clear, disp_head_d+pcb_head_d-clear, pcb_z])
            cube([r_w, pcb_d+2*clear, r_h]);
    }
}}

