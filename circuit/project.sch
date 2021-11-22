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
L ESP8285:ESP8285 U?
U 1 1 619417AD
P 3300 3650
F 0 "U?" H 4550 2300 50  0000 L CNN
F 1 "ESP8285" H 4550 2200 50  0000 L CNN
F 2 "Package_DFN_QFN:QFN50P500X500X90-33N-D" H 4750 4250 50  0001 L CNN
F 3 "http://espressif.com/sites/default/files/documentation/0a-esp8285_datasheet_en.pdf" H 4750 4150 50  0001 L CNN
F 4 "Wi-Fi SoC, qfn-32" H 4750 4050 50  0001 L CNN "Description"
F 5 "0.9" H 4750 3950 50  0001 L CNN "Height"
F 6 "Espressif Systems" H 4750 3850 50  0001 L CNN "Manufacturer_Name"
F 7 "ESP8285" H 4750 3750 50  0001 L CNN "Manufacturer_Part_Number"
F 8 "356-ESP8285" H 4750 3650 50  0001 L CNN "Mouser Part Number"
F 9 "https://www.mouser.co.uk/ProductDetail/Espressif-Systems/ESP8285?qs=chTDxNqvsyn49Ad%252BqWdhIQ%3D%3D" H 4750 3550 50  0001 L CNN "Mouser Price/Stock"
F 10 "" H 4750 3450 50  0001 L CNN "Arrow Part Number"
F 11 "" H 4750 3350 50  0001 L CNN "Arrow Price/Stock"
	1    3300 3650
	1    0    0    -1  
$EndComp
$Comp
L ESP8285:HX9193 U?
U 1 1 61962832
P 7800 4050
F 0 "U?" H 8075 4615 50  0000 C CNN
F 1 "HX9193" H 8075 4524 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-5" H 8050 3900 50  0001 C CNN
F 3 "" H 7800 4050 50  0001 C CNN
	1    7800 4050
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 619635D1
P 2800 4250
F 0 "R?" V 2593 4250 50  0000 C CNN
F 1 "R" V 2684 4250 50  0000 C CNN
F 2 "" V 2730 4250 50  0001 C CNN
F 3 "~" H 2800 4250 50  0001 C CNN
	1    2800 4250
	0    1    1    0   
$EndComp
Wire Wire Line
	2950 4250 3300 4250
$Comp
L power:+3.3V #PWR?
U 1 1 619642DB
P 8900 3250
F 0 "#PWR?" H 8900 3100 50  0001 C CNN
F 1 "+3.3V" H 8915 3423 50  0000 C CNN
F 2 "" H 8900 3250 50  0001 C CNN
F 3 "" H 8900 3250 50  0001 C CNN
	1    8900 3250
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 61964971
P 7100 3250
F 0 "#PWR?" H 7100 3100 50  0001 C CNN
F 1 "+5V" H 7115 3423 50  0000 C CNN
F 2 "" H 7100 3250 50  0001 C CNN
F 3 "" H 7100 3250 50  0001 C CNN
	1    7100 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	7100 3250 7100 3750
Wire Wire Line
	7100 3950 7500 3950
Wire Wire Line
	7500 3750 7100 3750
Connection ~ 7100 3750
Wire Wire Line
	7100 3750 7100 3950
$Comp
L Device:C C?
U 1 1 61965E62
P 7100 4350
F 0 "C?" H 7215 4396 50  0000 L CNN
F 1 "C" H 7215 4305 50  0000 L CNN
F 2 "" H 7138 4200 50  0001 C CNN
F 3 "~" H 7100 4350 50  0001 C CNN
	1    7100 4350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61966748
P 7400 4800
F 0 "#PWR?" H 7400 4550 50  0001 C CNN
F 1 "GND" H 7405 4627 50  0000 C CNN
F 2 "" H 7400 4800 50  0001 C CNN
F 3 "" H 7400 4800 50  0001 C CNN
	1    7400 4800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61968797
P 7100 4800
F 0 "#PWR?" H 7100 4550 50  0001 C CNN
F 1 "GND" H 7105 4627 50  0000 C CNN
F 2 "" H 7100 4800 50  0001 C CNN
F 3 "" H 7100 4800 50  0001 C CNN
	1    7100 4800
	1    0    0    -1  
$EndComp
Wire Wire Line
	7100 4500 7100 4800
Wire Wire Line
	7100 4200 7100 3950
Connection ~ 7100 3950
Wire Wire Line
	7400 4800 7400 3850
Wire Wire Line
	7400 3850 7500 3850
Wire Wire Line
	8650 3750 8900 3750
Wire Wire Line
	8900 3750 8900 3250
$Comp
L Device:C C?
U 1 1 6196AF89
P 8900 4350
F 0 "C?" H 9015 4396 50  0000 L CNN
F 1 "C" H 9015 4305 50  0000 L CNN
F 2 "" H 8938 4200 50  0001 C CNN
F 3 "~" H 8900 4350 50  0001 C CNN
	1    8900 4350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 6196B5F9
P 8900 4800
F 0 "#PWR?" H 8900 4550 50  0001 C CNN
F 1 "GND" H 8905 4627 50  0000 C CNN
F 2 "" H 8900 4800 50  0001 C CNN
F 3 "" H 8900 4800 50  0001 C CNN
	1    8900 4800
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 4800 8900 4500
Wire Wire Line
	8900 4200 8900 3750
Connection ~ 8900 3750
$Comp
L power:+3.3V #PWR?
U 1 1 61970094
P 2200 3650
F 0 "#PWR?" H 2200 3500 50  0001 C CNN
F 1 "+3.3V" V 2215 3778 50  0000 L CNN
F 2 "" H 2200 3650 50  0001 C CNN
F 3 "" H 2200 3650 50  0001 C CNN
	1    2200 3650
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2200 3650 3300 3650
$Comp
L power:+3.3V #PWR?
U 1 1 61971518
P 2200 4250
F 0 "#PWR?" H 2200 4100 50  0001 C CNN
F 1 "+3.3V" V 2215 4378 50  0000 L CNN
F 2 "" H 2200 4250 50  0001 C CNN
F 3 "" H 2200 4250 50  0001 C CNN
	1    2200 4250
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2200 4250 2650 4250
$Comp
L power:+3.3V #PWR?
U 1 1 6197218D
P 2200 3950
F 0 "#PWR?" H 2200 3800 50  0001 C CNN
F 1 "+3.3V" V 2215 4078 50  0000 L CNN
F 2 "" H 2200 3950 50  0001 C CNN
F 3 "" H 2200 3950 50  0001 C CNN
	1    2200 3950
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2200 3950 3100 3950
Wire Wire Line
	3300 3850 3100 3850
Wire Wire Line
	3100 3850 3100 3950
Connection ~ 3100 3950
Wire Wire Line
	3100 3950 3300 3950
$Comp
L Device:Crystal_GND24 Y?
U 1 1 619B0C73
P 7950 2000
F 0 "Y?" H 8144 2046 50  0000 L CNN
F 1 "26MHz" H 8144 1955 50  0000 L CNN
F 2 "Crystal:Crystal_SMD_3225-4Pin_3.2x2.5mm" H 7950 2000 50  0001 C CNN
F 3 "~" H 7950 2000 50  0001 C CNN
	1    7950 2000
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 619B191F
P 8500 2150
F 0 "C?" H 8615 2196 50  0000 L CNN
F 1 "C" H 8615 2105 50  0000 L CNN
F 2 "" H 8538 2000 50  0001 C CNN
F 3 "~" H 8500 2150 50  0001 C CNN
	1    8500 2150
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 619B1FED
P 7450 2150
F 0 "C?" H 7565 2196 50  0000 L CNN
F 1 "C" H 7565 2105 50  0000 L CNN
F 2 "" H 7488 2000 50  0001 C CNN
F 3 "~" H 7450 2150 50  0001 C CNN
	1    7450 2150
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 2000 7800 2000
Wire Wire Line
	8100 2000 8500 2000
$Comp
L power:GND #PWR?
U 1 1 619B3025
P 8500 2400
F 0 "#PWR?" H 8500 2150 50  0001 C CNN
F 1 "GND" H 8505 2227 50  0000 C CNN
F 2 "" H 8500 2400 50  0001 C CNN
F 3 "" H 8500 2400 50  0001 C CNN
	1    8500 2400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 619B36B2
P 7950 2400
F 0 "#PWR?" H 7950 2150 50  0001 C CNN
F 1 "GND" H 7955 2227 50  0000 C CNN
F 2 "" H 7950 2400 50  0001 C CNN
F 3 "" H 7950 2400 50  0001 C CNN
	1    7950 2400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 619B3ACC
P 7450 2400
F 0 "#PWR?" H 7450 2150 50  0001 C CNN
F 1 "GND" H 7455 2227 50  0000 C CNN
F 2 "" H 7450 2400 50  0001 C CNN
F 3 "" H 7450 2400 50  0001 C CNN
	1    7450 2400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 619B3FC4
P 7950 1550
F 0 "#PWR?" H 7950 1300 50  0001 C CNN
F 1 "GND" H 7955 1377 50  0000 C CNN
F 2 "" H 7950 1550 50  0001 C CNN
F 3 "" H 7950 1550 50  0001 C CNN
	1    7950 1550
	-1   0    0    1   
$EndComp
Wire Wire Line
	7950 1550 7950 1800
Wire Wire Line
	7950 2200 7950 2400
Wire Wire Line
	8500 2400 8500 2300
Wire Wire Line
	7450 2400 7450 2300
Text Label 4200 2600 1    50   ~ 0
XI
Text Label 4300 2600 1    50   ~ 0
XO
Wire Wire Line
	4300 2600 4300 2850
Wire Wire Line
	4200 2600 4200 2850
Text Label 7450 2000 0    50   ~ 0
XI
Text Label 8400 2000 0    50   ~ 0
XO
$Comp
L power:+3.3V #PWR?
U 1 1 619C402A
P 4100 1750
F 0 "#PWR?" H 4100 1600 50  0001 C CNN
F 1 "+3.3V" V 4115 1878 50  0000 L CNN
F 2 "" H 4100 1750 50  0001 C CNN
F 3 "" H 4100 1750 50  0001 C CNN
	1    4100 1750
	1    0    0    -1  
$EndComp
Wire Wire Line
	4100 1750 4100 2400
Wire Wire Line
	4000 2850 4000 2400
Wire Wire Line
	4000 2400 4100 2400
Connection ~ 4100 2400
Wire Wire Line
	4100 2400 4100 2850
$Comp
L power:GND #PWR?
U 1 1 619C71F1
P 3700 1750
F 0 "#PWR?" H 3700 1500 50  0001 C CNN
F 1 "GND" H 3705 1577 50  0000 C CNN
F 2 "" H 3700 1750 50  0001 C CNN
F 3 "" H 3700 1750 50  0001 C CNN
	1    3700 1750
	-1   0    0    1   
$EndComp
Wire Wire Line
	3700 1750 3700 2850
$EndSCHEMATC
