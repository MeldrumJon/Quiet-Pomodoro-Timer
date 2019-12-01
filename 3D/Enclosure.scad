include <bat.scad>
include <disp.scad>
include <enc.scad>
include <pcb.scad>

$fa = 0.2;
$fs = 0.2;

clr = 0.5;

pcb_x = 4.65;
pcb_y = disp_head_d+pcb_head_d;
pcb_z = 30.8-pcb_h;

// innards
translate([0, 7.28, 0]) union() {
    translate([0, 55, 0]) rotate([90, 0, 0]) bat();
    
    translate([2, 0, 20]) rotate([-25, 0, 0])
    union() {
        disp_components();
        disp_holder();
    }
}



module disp_components() { union() {
    rotate([90, 0, 0]) disp();
    translate([pcb_x, pcb_y, pcb_h+pcb_z]) 
        rotate([-90, 0, 0]) color("purple") pcb();
}}

module disp_holder() { union() {
    d = 2.4; // Make large enough to connect to viewport
    disp_olap = 8;

    // Display legs
    difference() {
        translate([-d, -(d+disp_d), -d]) 
            cube([disp_w+2*d, disp_d+2*d, d+disp_olap]);
        translate([-clr, -(disp_d+clr), -clr])
            cube([disp_w+2*clr, disp_d+2*clr, disp_h]);
        translate([disp_screen_beg_x-clr, -50, -(d+0.1)])
            cube([disp_screen_w+2*clr, 100, disp_screen_h]);
    }

    // PCB legs
    pcb_olap_l = 2;
    pcb_olap_r = 5;
    pcb_w_l = 10;
    pcb_w_r = 5;
    translate([pcb_x, pcb_y, pcb_z])
    difference() {
        union() {
            translate([-d, -d, -d])
                cube([pcb_w/2+2*d, pcb_d+2*d, d+pcb_olap_l]);
            translate([pcb_w/2-d, -d, -d])
                cube([pcb_w/2+2*d, pcb_d+2*d, d+pcb_olap_r]);
        }
        translate([-clr, -clr, -clr])
            cube([pcb_w+2*clr, pcb_d+2*clr, pcb_h]);
        translate([pcb_w_l, -50, -(d+0.1)])
            cube([pcb_w-(pcb_w_l+pcb_w_r), 100, pcb_h]); 
    }

    w = 1;
    // Viewport
    difference() {
        translate([-d, -(disp_d+disp_screen_d+w+clr), -d])
            cube([disp_w+2*d, w, disp_h+d]);
        translate([disp_view_beg_x, -50, disp_view_beg_y])
            cube([disp_view_w, 100, disp_view_h]);
    }

}}
