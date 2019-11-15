$fa = 0.2;
$fs = 0.2;

pcb_w = 40;
pcb_h = 26;
pcb_d = 1.6;

pcb_head_d = 4.2 - pcb_d;

module pcb() {
    translate([pcb_w/2, pcb_h/2, pcb_d]) import("res/quietTimerPCB.stl");
}

// pcb();
