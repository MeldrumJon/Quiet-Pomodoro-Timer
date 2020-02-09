include <lib/BOSL/constants.scad>
use <lib/BOSL/shapes.scad>
include <bat.scad>
include <disp.scad>
include <enc.scad>
include <pcb.scad>

$fa = 0.02;
$fs = 0.02;


insert_h = 10*1.5+2;
insert_d = 5+2;

wall = 1.6;

clr = 0.5;

pcb_x = 4.65;
pcb_y = disp_head_d+pcb_head_d;
pcb_z = 30.8-pcb_h;

stand_d = 2.4;
view_w = stand_d*2 + disp_w;

view_x = [wall+insert_d+clr+(bat_h-view_w)/2, wall+insert_d+clr+(bat_h-view_w)/2+view_w];
view_y = [0, sin(25)*(disp_view_h+18+clr)];

difference() {
    union() {
        front();
        //back();
    }
}

module front() {
    translate([wall+insert_d+clr, 0, 0]) union() {
        translate([0, bat_d+44, bat_w+wall/2]) rotate([90, 90, 0]) bat();
        
    
        translate([(bat_h-view_w)/2, 0, 16]) rotate([-25, 0, 0])
        union() {
            disp_components();
            disp_holder();
        }
    }
    translate([wall+insert_d/2, 44+bat_d-insert_h+clr, wall+insert_d/2]) rotate([-90, 0, 0]) insert_hole();
    translate([bat_h+wall+insert_d+clr+insert_d/2+clr, 44+bat_d-insert_h+clr, wall+insert_d/2]) rotate([-90, 0, 0]) insert_hole();
    translate([0, 0, bat_w-wall/2+clr]) {
        translate([wall+insert_d/2, 44+bat_d-insert_h+clr, wall+insert_d/2]) rotate([-90, 0, 0]) insert_hole();
        translate([bat_h+wall+insert_d+clr+insert_d/2+clr, 44+bat_d-insert_h+clr, wall+insert_d/2]) rotate([-90, 0, 0]) insert_hole();
    }

    wall_bh = 16;
    wall_th = wall/2+bat_w+insert_d+clr;

    linear_extrude(wall_bh) difference() {
        base(0);
        base(wall, 1);
    }
    linear_extrude(wall) base(0); // floor

    polyhedron(points=[
        [view_x[0], view_y[0], wall_bh],
        [view_x[0], view_y[0]+wall+1, wall_bh],
        [wall+1, 44, wall_bh],
        [0, 44, wall_bh],
        [view_x[0], view_y[1], wall_th],
        [view_x[0], view_y[1]+wall+1, wall_th],
        [wall+1, 44, wall_th],
        [0, 44, wall_th],
    ],
    faces=[
        [0,1,2,3],  // bottom
        [4,5,1,0],  // front
        [7,6,5,4],  // top
        [5,6,2,1],  // right
        [6,7,3,2],  // back
        [7,4,0,3] // left
    ]);
}

module back() {
    translate([0, 0, bat_w+wall/2+insert_d+clr]) linear_extrude(wall) base(1);
}

module base(sub=0, ext=0) {
    polygon(points=[
        [0+sub, 44],
        [(bat_h-view_w)/2+wall+insert_d+clr+sub, 0+sub],
        [(bat_h-view_w)/2+wall+insert_d+clr+view_w-sub, 0+sub],
        [wall+insert_d+clr+bat_h+clr+insert_d+wall-sub, 44]
    ]);
    translate([sub, 44, 0]) square([(wall+clr+insert_d)*2+bat_h-sub*2, bat_d+clr+ext]);
}

module disp_components() { translate([stand_d, disp_d+disp_screen_d+1+clr, stand_d+5-clr/2]) union() {
    rotate([90, 0, 0]) disp();
    translate([pcb_x, pcb_y, pcb_h+pcb_z]) 
        rotate([-90, 0, 0]) color("purple") pcb();
}}

module disp_holder() { translate([stand_d, disp_d+disp_screen_d+1+clr, stand_d+5-clr/2]) union() {
    d = stand_d; // Make large enough to connect to viewport
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
    extra = 18+clr;
    // Viewport
    difference() {
        translate([-d, -(disp_d+disp_screen_d+w+clr), d-0.3-extra/2])
            cube([disp_w+2*d, w, disp_view_h+extra]);
        translate([disp_view_beg_x, -50, disp_view_beg_y])
            cube([disp_view_w, 100, disp_view_h]);
    }

}}


module insert_hole() { // M3x10x5 inserts
    id = 5;
    ih = 10;
    h = ih*1.5+2;
    difference() {
        cylinder(d=id+2, h=h);
        translate([0, 0, h+0.01]) tapered_insert_hole(h=ih*1.5, d=id-0.5, taper_h=2, taper_d=id+clr);
    }
}

module tapered_insert_hole(h, d, taper_h, taper_d) {
    union() {
        translate([0, 0, -h]) cylinder(h=h, d=d);
        translate([0, 0, -taper_h/2]) cyl(h=taper_h, d1=d, d2=taper_d);
    }
}

