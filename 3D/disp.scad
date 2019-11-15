$fa = 0.2;
$fs = 0.2;

disp_w = 44.50;
disp_h = 37;
disp_d = 1.8;

disp_hole_r = 3.0 / 2;
disp_hole_off = 2.5;

disp_conn_w = 7.55;
disp_conn_h = 18;
disp_conn_d = 7.25 - disp_d;

disp_conn_beg_x = 0;
disp_conn_end_x = disp_conn_beg_x + disp_conn_w;
disp_conn_beg_y = 10;
disp_conn_end_y = disp_conn_beg_y + disp_conn_h;

disp_head_w = 2.46;
disp_head_h = 19.05;
disp_head_d = 10.25;

disp_head_beg_x = 41.5;
disp_head_end_x = disp_head_beg_x + disp_head_w;
disp_head_beg_y = 9.85;
disp_head_end_y = disp_head_beg_y + disp_head_h;

disp_screen_beg_x = 5.2;
disp_screen_end_x = 39.2;
disp_screen_beg_y = 0;
disp_screen_end_y = disp_h - 3.1;

disp_screen_d = 3.5 - disp_d;
disp_screen_w = disp_screen_end_x - disp_screen_beg_x;
disp_screen_h = disp_screen_end_y - disp_screen_beg_y;

// Recommended clearance for an outer case
disp_view_clearance = 0.6;

disp_view_beg_x = 8.5;
disp_view_end_x = 35.5;
disp_view_beg_y = 2.1;
disp_view_end_y = 29;

disp_view_w = disp_view_end_x - disp_view_beg_x;
disp_view_h = disp_view_end_y - disp_view_beg_y;

module disp() {
    // PCB
    difference() {
        color("blue") cube([disp_w, disp_h, disp_d]);

        for (x=[0+disp_hole_off, disp_w-disp_hole_off]) {
            for (y=[0+disp_hole_off, disp_h-disp_hole_off]) {
                translate([x, y, -0.1]) cylinder(h=disp_d+0.2, r=disp_hole_r);
            }
        }
    }

    // Screen
    color("#555555")
        translate([disp_screen_beg_x, disp_screen_beg_y, disp_d])
            cube([disp_screen_w, disp_screen_h, disp_screen_d]);

    // View
    color("white")
        translate([disp_view_beg_x, disp_view_beg_y, disp_d+disp_screen_d])
            cube([disp_view_w, disp_view_h, 0.1]);

    // Headers
    color("#333333")
        translate([disp_head_beg_x, disp_head_beg_y, -disp_head_d])
            cube([disp_head_w, disp_head_h, disp_head_d]);

    // Connector
    color("tan")
        translate([disp_conn_beg_x, disp_conn_beg_y, -disp_conn_d])
            cube([disp_conn_w, disp_conn_h, disp_conn_d]);

}

// disp();
