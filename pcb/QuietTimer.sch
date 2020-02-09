EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MCU_Microchip_ATmega:ATmega328P-PU U?
U 1 1 5E408BE9
P 3850 3400
F 0 "U?" H 3206 3446 50  0000 R CNN
F 1 "ATmega328P-PU" H 3206 3355 50  0000 R CNN
F 2 "Package_DIP:DIP-28_W7.62mm" H 3850 3400 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/ATmega328_P%20AVR%20MCU%20with%20picoPower%20Technology%20Data%20Sheet%2040001984A.pdf" H 3850 3400 50  0001 C CNN
	1    3850 3400
	1    0    0    -1  
$EndComp
$Comp
L Device:Rotary_Encoder_Switch SW?
U 1 1 5E40A3D6
P 6550 1850
F 0 "SW?" V 6504 2080 50  0000 L CNN
F 1 "Rotary_Encoder_Switch" V 6595 2080 50  0000 L CNN
F 2 "" H 6400 2010 50  0001 C CNN
F 3 "~" H 6550 2110 50  0001 C CNN
	1    6550 1850
	0    1    1    0   
$EndComp
$Comp
L Connector:AVR-ISP-6 J?
U 1 1 5E40B3A3
P 7450 3300
F 0 "J?" H 7171 3396 50  0000 R CNN
F 1 "AVR-ISP-6" H 7171 3305 50  0000 R CNN
F 2 "" V 7200 3350 50  0001 C CNN
F 3 " ~" H 6175 2750 50  0001 C CNN
	1    7450 3300
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x02_Female J?
U 1 1 5E40BE71
P 5450 2950
F 0 "J?" H 5478 2926 50  0000 L CNN
F 1 "Conn_01x02_Female" H 5478 2835 50  0000 L CNN
F 2 "" H 5450 2950 50  0001 C CNN
F 3 "~" H 5450 2950 50  0001 C CNN
	1    5450 2950
	1    0    0    -1  
$EndComp
$EndSCHEMATC
