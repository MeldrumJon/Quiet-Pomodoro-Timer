enc_pcb_w = 26;
enc_pcb_h = 16;
enc_pcb_d = 1.6;

enc_pole_r = 7 / 2.0;
enc_pole_h = 28.6;

enc_box_w = 12.5;
enc_box_h = 12.5;
enc_box_d = 8.5 - enc_pcb_d;

module enc() {
    translate([0, 0, enc_pcb_d])
        color("purple") import("res/rotaryEncoderPCB.stl");
    translate([-enc_box_w/2, -enc_box_h/2, enc_pcb_d])
        color("silver") cube([enc_box_w, enc_box_h, enc_box_d]);    
    translate([0, 0, 0.1])
        color("silver") cylinder(r=enc_pole_r, h=enc_pole_h-0.1);
}

//enc();
