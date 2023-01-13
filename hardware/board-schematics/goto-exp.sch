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
L Device:CP_Small C2
U 1 1 625F3EC5
P 5000 5450
F 0 "C2" V 4775 5450 50  0000 C CNN
F 1 "100 uF" V 4866 5450 50  0000 C CNN
F 2 "Capacitor_THT:CP_Radial_D10.0mm_P5.00mm" H 5000 5450 50  0001 C CNN
F 3 "~" H 5000 5450 50  0001 C CNN
	1    5000 5450
	0    1    1    0   
$EndComp
$Comp
L Device:CP_Small C1
U 1 1 625F4AC9
P 2900 5450
F 0 "C1" V 2675 5450 50  0000 C CNN
F 1 "100 uF" V 2766 5450 50  0000 C CNN
F 2 "Capacitor_THT:CP_Radial_D10.0mm_P5.00mm" H 2900 5450 50  0001 C CNN
F 3 "~" H 2900 5450 50  0001 C CNN
	1    2900 5450
	0    1    1    0   
$EndComp
$Comp
L Connector:Conn_Coaxial_Power J1
U 1 1 625F5B4C
P 5500 6250
F 0 "J1" H 5588 6246 50  0000 L CNN
F 1 "Conn_Coaxial_Power" H 5588 6155 50  0001 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 5500 6200 50  0001 C CNN
F 3 "~" H 5500 6200 50  0001 C CNN
	1    5500 6250
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x04_Male Arduino1
U 1 1 625FE966
P 4450 1350
F 0 "Arduino1" V 4512 1494 50  0000 L CNN
F 1 "Conn_01x04_Male" V 4603 1494 50  0001 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 4450 1350 50  0001 C CNN
F 3 "~" H 4450 1350 50  0001 C CNN
	1    4450 1350
	0    1    1    0   
$EndComp
$Comp
L Connector:Conn_01x04_Male ALTMotor1
U 1 1 6261AF35
P 2500 6000
F 0 "ALTMotor1" V 2654 5712 50  0000 R CNN
F 1 "Conn_01x04_Male" V 2563 5712 50  0001 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 2500 6000 50  0001 C CNN
F 3 "~" H 2500 6000 50  0001 C CNN
	1    2500 6000
	0    -1   -1   0   
$EndComp
$Comp
L Connector:Conn_01x04_Male AZMotor1
U 1 1 6261BCE5
P 4600 6000
F 0 "AZMotor1" V 4754 5712 50  0000 R CNN
F 1 "Conn_01x04_Male" V 4663 5712 50  0001 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 4600 6000 50  0001 C CNN
F 3 "~" H 4600 6000 50  0001 C CNN
	1    4600 6000
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5500 6150 5350 6150
Wire Wire Line
	5100 6150 5100 5450
Wire Wire Line
	3000 6150 3000 5450
Connection ~ 5100 6150
Wire Wire Line
	5500 6450 4900 6450
Wire Wire Line
	4900 6450 4900 5450
Wire Wire Line
	2800 6450 2800 5450
Connection ~ 4900 6450
$Comp
L Driver_Motor:Pololu_Breakout_DRV8825 StepperDriver1
U 1 1 62688C03
P 2700 4550
F 0 "StepperDriver1" V 2654 5194 50  0000 L CNN
F 1 "Pololu_Breakout_DRV8825" V 2745 5194 50  0001 L CNN
F 2 "Module:Pololu_Breakout-16_15.2x20.3mm" H 2900 3750 50  0001 L CNN
F 3 "https://www.pololu.com/product/2982" H 2800 4250 50  0001 C CNN
	1    2700 4550
	0    1    1    0   
$EndComp
$Comp
L Driver_Motor:Pololu_Breakout_DRV8825 StepperDriver2
U 1 1 626B5835
P 4750 4500
F 0 "StepperDriver2" V 4704 5144 50  0000 L CNN
F 1 "Pololu_Breakout_DRV8825" V 4795 5144 50  0001 L CNN
F 2 "Module:Pololu_Breakout-16_15.2x20.3mm" H 4950 3700 50  0001 L CNN
F 3 "https://www.pololu.com/product/2982" H 4850 4200 50  0001 C CNN
	1    4750 4500
	0    1    1    0   
$EndComp
Wire Wire Line
	2600 5800 2600 5050
Wire Wire Line
	2500 5800 2500 4950
Wire Wire Line
	2400 5800 2400 4950
Wire Wire Line
	4800 5800 4800 5100
Wire Wire Line
	4700 5800 4700 5000
Wire Wire Line
	4600 5800 4600 5050
Wire Wire Line
	4600 5050 4550 5050
Wire Wire Line
	4550 5050 4550 4900
Wire Wire Line
	4500 5800 4500 5000
Wire Wire Line
	4500 5000 4450 5000
Wire Wire Line
	4450 5000 4450 4900
Wire Wire Line
	3000 4150 2900 4150
Wire Wire Line
	5050 4100 4950 4100
$Comp
L Connector_Generic:Conn_02x20_Odd_Even RPi1
U 1 1 625EC189
P 3650 2650
F 0 "RPi1" V 3746 1563 50  0000 R CNN
F 1 "Conn_02x20" V 3655 1563 50  0001 R CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x20_P2.54mm_Vertical" H 3650 2650 50  0001 C CNN
F 3 "~" H 3650 2650 50  0001 C CNN
	1    3650 2650
	0    1    -1   0   
$EndComp
Wire Wire Line
	2500 4150 2500 3200
Wire Wire Line
	2500 3200 2750 3200
Wire Wire Line
	2750 3200 2750 2850
Wire Wire Line
	2600 4150 2600 3300
Wire Wire Line
	2100 4150 2100 3750
Wire Wire Line
	3850 3400 3850 2850
Wire Wire Line
	2200 4150 2200 3500
Wire Wire Line
	2200 3500 3950 3500
Wire Wire Line
	3950 3500 3950 2850
Wire Wire Line
	4050 3650 4050 2850
NoConn ~ 4150 2850
NoConn ~ 3750 2850
NoConn ~ 3650 2850
NoConn ~ 3550 2850
NoConn ~ 3450 2850
NoConn ~ 3350 2850
NoConn ~ 3250 2850
NoConn ~ 3150 2850
NoConn ~ 3050 2850
NoConn ~ 4550 2350
NoConn ~ 4050 2350
NoConn ~ 3950 2350
NoConn ~ 3850 2350
NoConn ~ 3750 2350
NoConn ~ 3650 2350
NoConn ~ 3250 2350
NoConn ~ 3150 2350
NoConn ~ 2950 2350
NoConn ~ 2750 2350
NoConn ~ 2650 2350
NoConn ~ 3100 4150
NoConn ~ 5150 4100
NoConn ~ 2700 4150
NoConn ~ 4750 4100
Wire Wire Line
	4550 4100 4550 3250
Wire Wire Line
	4650 4100 4650 3450
Wire Wire Line
	4650 3450 5100 3450
Wire Wire Line
	5100 3450 5100 1950
Wire Wire Line
	5100 1950 3050 1950
Wire Wire Line
	3050 1950 3050 2350
Wire Wire Line
	4350 4100 4350 3100
Wire Wire Line
	1900 4550 1900 3800
Wire Wire Line
	1900 2850 2650 2850
Wire Wire Line
	3950 4500 3950 3800
Wire Wire Line
	3950 3800 1900 3800
Connection ~ 1900 3800
Wire Wire Line
	1900 3800 1900 2850
Wire Wire Line
	1900 4650 1900 6450
Wire Wire Line
	1900 6450 2800 6450
Connection ~ 2800 6450
Wire Wire Line
	4950 4100 4950 3950
Wire Wire Line
	4950 3600 5250 3600
Wire Wire Line
	5250 3600 5250 2850
Wire Wire Line
	5250 2850 4550 2850
Connection ~ 4950 4100
Wire Wire Line
	3000 4150 3000 3950
Wire Wire Line
	3000 3950 4950 3950
Connection ~ 3000 4150
Connection ~ 4950 3950
Wire Wire Line
	4950 3950 4950 3600
Wire Wire Line
	4450 2350 4450 1750
Wire Wire Line
	4450 1750 4550 1750
Wire Wire Line
	4550 1750 4550 1550
Wire Wire Line
	4350 2350 4350 1700
Wire Wire Line
	4350 1700 4450 1700
Wire Wire Line
	4450 1700 4450 1550
Wire Wire Line
	4250 2350 4250 1650
Wire Wire Line
	4250 1650 4350 1650
Wire Wire Line
	4350 1650 4350 1550
Wire Wire Line
	4150 2350 4150 1550
Wire Wire Line
	4150 1550 4250 1550
Wire Wire Line
	3300 4550 3300 6150
Connection ~ 3300 6150
Wire Wire Line
	3300 6150 3000 6150
Wire Wire Line
	5350 4500 5350 6150
Connection ~ 5350 6150
Wire Wire Line
	5350 6150 5100 6150
Wire Wire Line
	2700 4950 2700 5800
Wire Wire Line
	2600 5050 2800 5050
Wire Wire Line
	2800 5050 2800 4950
Wire Wire Line
	4750 5100 4800 5100
Wire Wire Line
	4750 4900 4750 5100
Wire Wire Line
	4700 5000 4850 5000
Wire Wire Line
	4850 5000 4850 4900
Wire Wire Line
	4150 3000 4150 4100
Wire Wire Line
	2300 3400 3850 3400
Wire Wire Line
	2300 3400 2300 4150
Wire Wire Line
	2100 3750 2400 3750
Wire Wire Line
	2400 3750 2400 3650
Wire Wire Line
	2400 3650 4050 3650
Wire Wire Line
	2950 3300 2950 2850
Wire Wire Line
	2600 3300 2950 3300
Wire Wire Line
	4250 3050 4350 3050
Wire Wire Line
	4350 3050 4350 2850
Wire Wire Line
	4250 3050 4250 4100
Wire Wire Line
	4550 3250 2850 3250
Wire Wire Line
	2850 3250 2850 2850
NoConn ~ 2850 2350
NoConn ~ 3350 2350
NoConn ~ 3450 2350
NoConn ~ 3550 2350
Wire Wire Line
	4450 3000 4450 2850
Wire Wire Line
	4150 3000 4450 3000
Wire Wire Line
	4350 3100 4400 3100
Wire Wire Line
	4400 3100 4400 2900
Wire Wire Line
	4400 2900 4250 2900
Wire Wire Line
	4250 2900 4250 2850
Wire Wire Line
	2800 6450 3950 6450
Wire Wire Line
	3300 6150 5100 6150
Wire Wire Line
	3950 4600 3950 6450
Connection ~ 3950 6450
Wire Wire Line
	3950 6450 4900 6450
$Comp
L teensy:Teensy4.0 U1
U 1 1 62CB730D
P 8350 2300
F 0 "U1" H 8350 3915 50  0000 C CNN
F 1 "Teensy4.0" H 8350 3824 50  0000 C CNN
F 2 "teensy:Teensy40" H 7950 2500 50  0001 C CNN
F 3 "" H 7950 2500 50  0001 C CNN
	1    8350 2300
	1    0    0    -1  
$EndComp
Connection ~ 4450 1550
Wire Wire Line
	9450 3350 10150 3350
Wire Wire Line
	4550 1550 4550 1250
Wire Wire Line
	4550 1250 6450 1250
Wire Wire Line
	6450 1250 6450 4800
Wire Wire Line
	6450 4800 10400 4800
Wire Wire Line
	10400 4800 10400 3250
Wire Wire Line
	10400 3250 9450 3250
Connection ~ 4550 1550
$Comp
L Connector:Conn_01x05_Male J2
U 1 1 62CEF15D
P 10650 1600
F 0 "J2" H 10622 1624 50  0000 R CNN
F 1 "Conn_01x05_Male" H 10622 1533 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x05_P2.54mm_Vertical" H 10650 1600 50  0001 C CNN
F 3 "~" H 10650 1600 50  0001 C CNN
	1    10650 1600
	-1   0    0    -1  
$EndComp
Wire Wire Line
	10450 1400 10250 1400
Wire Wire Line
	10450 1500 10150 1500
Wire Wire Line
	10150 1500 10150 3350
Wire Wire Line
	7250 2850 6850 2850
Wire Wire Line
	6850 2850 6850 4250
Wire Wire Line
	6850 4250 10550 4250
Wire Wire Line
	10550 4250 10550 3050
Wire Wire Line
	10550 3050 10250 3050
Wire Wire Line
	9700 4000 7100 4000
Wire Wire Line
	7100 4000 7100 3050
Wire Wire Line
	7100 3050 7250 3050
Wire Wire Line
	10450 1700 9800 1700
Wire Wire Line
	9800 1700 9800 4100
Wire Wire Line
	9800 4100 7000 4100
Wire Wire Line
	7000 4100 7000 2950
Wire Wire Line
	7000 2950 7250 2950
Wire Wire Line
	10050 1400 10050 3450
Wire Wire Line
	4450 950  4450 1550
Wire Wire Line
	7250 1050 4350 1050
Wire Wire Line
	4350 1050 4350 1550
Connection ~ 4350 1550
Wire Wire Line
	7250 1150 4250 1150
Wire Wire Line
	4250 1150 4250 1550
Connection ~ 4250 1550
Wire Wire Line
	4450 950  6100 950 
Wire Wire Line
	6100 950  6100 4500
Wire Wire Line
	6100 4500 10850 4500
Wire Wire Line
	10850 4500 10850 3350
Wire Wire Line
	10850 3350 10150 3350
Connection ~ 10150 3350
$Comp
L Connector:Conn_01x04_Male J3
U 1 1 62D72CBD
P 10150 650
F 0 "J3" H 10212 794 50  0000 L CNN
F 1 "Conn_01x04_Male" V 10100 0   50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 10150 650 50  0001 C CNN
F 3 "~" H 10150 650 50  0001 C CNN
	1    10150 650 
	0    1    1    0   
$EndComp
Wire Wire Line
	10250 850  10250 1050
Connection ~ 10250 1400
Wire Wire Line
	10250 1400 10050 1400
Wire Wire Line
	10150 1500 10150 1100
Connection ~ 10150 1500
Wire Wire Line
	7250 3450 7150 3450
Wire Wire Line
	7150 3450 7150 3900
Wire Wire Line
	10000 3900 10000 1150
Wire Wire Line
	10000 1100 10050 1100
Wire Wire Line
	10050 1100 10050 850 
Wire Wire Line
	7250 3350 6700 3350
Wire Wire Line
	6700 3350 6700 4400
Wire Wire Line
	9900 1000 9950 1000
Wire Wire Line
	9950 1000 9950 850 
Wire Wire Line
	9700 1600 9700 4000
Wire Wire Line
	9700 1600 10450 1600
Wire Wire Line
	10250 3050 10250 1800
Wire Wire Line
	10250 1800 10450 1800
$Comp
L Connector:Conn_01x04_Male J4
U 1 1 63251DC9
P 10700 650
F 0 "J4" H 10762 794 50  0000 L CNN
F 1 "Conn_01x04_Male" V 10650 350 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 10700 650 50  0001 C CNN
F 3 "~" H 10700 650 50  0001 C CNN
	1    10700 650 
	0    1    1    0   
$EndComp
Wire Wire Line
	9900 4400 9900 1200
Wire Wire Line
	10500 850  10500 1200
Wire Wire Line
	10500 1200 9900 1200
Connection ~ 9900 1200
Wire Wire Line
	9900 1200 9900 1000
Wire Wire Line
	10600 850  10600 1150
Wire Wire Line
	10600 1150 10000 1150
Connection ~ 10000 1150
Wire Wire Line
	10000 1150 10000 1100
Wire Wire Line
	10700 850  10700 1100
Wire Wire Line
	10700 1100 10150 1100
Connection ~ 10150 1100
Wire Wire Line
	10150 1100 10150 850 
Wire Wire Line
	10800 850  10800 1050
Wire Wire Line
	10800 1050 10250 1050
Connection ~ 10250 1050
Wire Wire Line
	10250 1050 10250 1400
Wire Wire Line
	7150 3900 10000 3900
Wire Wire Line
	6700 4400 9900 4400
Wire Wire Line
	9450 3450 10050 3450
$EndSCHEMATC
