EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 6 11
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
L Relay:G5Q-1A K1
U 1 1 5F96D90B
P 5600 3950
AR Path="/5F96B8A3/5F96D90B" Ref="K1"  Part="1" 
AR Path="/5F982F80/5F96D90B" Ref="K2"  Part="1" 
F 0 "K1" H 5930 3996 50  0000 L CNN
F 1 "G5Q-1A" H 5930 3905 50  0000 L CNN
F 2 "Relay_THT:Relay_SPST_Omron-G5Q-1A" H 5950 3900 50  0001 L CNN
F 3 "https://www.omron.com/ecb/products/pdf/en-g5q.pdf" H 5600 3950 50  0001 C CNN
	1    5600 3950
	1    0    0    -1  
$EndComp
$Comp
L Transistor_FET:2N7002 Q1
U 1 1 5F96DF25
P 5300 4450
AR Path="/5F96B8A3/5F96DF25" Ref="Q1"  Part="1" 
AR Path="/5F982F80/5F96DF25" Ref="Q2"  Part="1" 
F 0 "Q1" H 5504 4496 50  0000 L CNN
F 1 "RK7002BM" H 5504 4405 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 5500 4375 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N7002.pdf" H 5300 4450 50  0001 L CNN
	1    5300 4450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR062
U 1 1 5F973550
P 5400 4650
AR Path="/5F96B8A3/5F973550" Ref="#PWR062"  Part="1" 
AR Path="/5F982F80/5F973550" Ref="#PWR065"  Part="1" 
F 0 "#PWR065" H 5400 4400 50  0001 C CNN
F 1 "GND" H 5405 4477 50  0000 C CNN
F 2 "" H 5400 4650 50  0001 C CNN
F 3 "" H 5400 4650 50  0001 C CNN
	1    5400 4650
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR061
U 1 1 5F9738CA
P 5400 3650
AR Path="/5F96B8A3/5F9738CA" Ref="#PWR061"  Part="1" 
AR Path="/5F982F80/5F9738CA" Ref="#PWR064"  Part="1" 
F 0 "#PWR064" H 5400 3500 50  0001 C CNN
F 1 "+5V" H 5415 3823 50  0000 C CNN
F 2 "" H 5400 3650 50  0001 C CNN
F 3 "" H 5400 3650 50  0001 C CNN
	1    5400 3650
	1    0    0    -1  
$EndComp
Text HLabel 4900 4450 0    50   Input ~ 0
RELAY
$Comp
L Connector:Screw_Terminal_01x02 J15
U 1 1 5F974177
P 6650 3900
AR Path="/5F96B8A3/5F974177" Ref="J15"  Part="1" 
AR Path="/5F982F80/5F974177" Ref="J16"  Part="1" 
F 0 "J15" H 6730 3892 50  0000 L CNN
F 1 "Switched" H 6730 3801 50  0000 L CNN
F 2 "" H 6650 3900 50  0001 C CNN
F 3 "~" H 6650 3900 50  0001 C CNN
	1    6650 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	6450 3650 6450 3900
Wire Wire Line
	5800 4250 6450 4250
Wire Wire Line
	6450 4250 6450 4000
$Comp
L Device:D D10
U 1 1 5F974CD1
P 4900 3950
AR Path="/5F96B8A3/5F974CD1" Ref="D10"  Part="1" 
AR Path="/5F982F80/5F974CD1" Ref="D12"  Part="1" 
F 0 "D10" H 4850 4050 50  0000 L CNN
F 1 "S1GTR" H 4800 3850 50  0000 L CNN
F 2 "Diodes_SMD:SMA_Standard" H 4900 3950 50  0001 C CNN
F 3 "~" H 4900 3950 50  0001 C CNN
	1    4900 3950
	0    1    1    0   
$EndComp
Wire Wire Line
	5400 4250 4900 4250
Wire Wire Line
	4900 4250 4900 4100
Connection ~ 5400 4250
Wire Wire Line
	4900 3800 4900 3650
Wire Wire Line
	4900 3650 5400 3650
Connection ~ 5400 3650
$Comp
L Device:LED D11
U 1 1 5F975582
P 5000 5050
AR Path="/5F96B8A3/5F975582" Ref="D11"  Part="1" 
AR Path="/5F982F80/5F975582" Ref="D13"  Part="1" 
F 0 "D11" V 5039 4932 50  0000 R CNN
F 1 "RED" V 4948 4932 50  0000 R CNN
F 2 "LEDs:LED_0603" H 5000 5050 50  0001 C CNN
F 3 "~" H 5000 5050 50  0001 C CNN
	1    5000 5050
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R10
U 1 1 5F975CB8
P 5000 4750
AR Path="/5F96B8A3/5F975CB8" Ref="R10"  Part="1" 
AR Path="/5F982F80/5F975CB8" Ref="R11"  Part="1" 
F 0 "R10" H 5070 4796 50  0000 L CNN
F 1 "680R" H 5070 4705 50  0000 L CNN
F 2 "Resistors_SMD:R_0603" V 4930 4750 50  0001 C CNN
F 3 "~" H 5000 4750 50  0001 C CNN
	1    5000 4750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR063
U 1 1 5F975F75
P 5000 5200
AR Path="/5F96B8A3/5F975F75" Ref="#PWR063"  Part="1" 
AR Path="/5F982F80/5F975F75" Ref="#PWR066"  Part="1" 
F 0 "#PWR066" H 5000 4950 50  0001 C CNN
F 1 "GND" H 5005 5027 50  0000 C CNN
F 2 "" H 5000 5200 50  0001 C CNN
F 3 "" H 5000 5200 50  0001 C CNN
	1    5000 5200
	1    0    0    -1  
$EndComp
Wire Wire Line
	4900 4450 5000 4450
Wire Wire Line
	5000 4600 5000 4450
Connection ~ 5000 4450
Wire Wire Line
	5000 4450 5100 4450
$Comp
L Device:Fuse F1
U 1 1 5F9F79BC
P 6300 3650
AR Path="/5F96B8A3/5F9F79BC" Ref="F1"  Part="1" 
AR Path="/5F982F80/5F9F79BC" Ref="F2"  Part="1" 
F 0 "F1" V 6103 3650 50  0000 C CNN
F 1 "10A" V 6194 3650 50  0000 C CNN
F 2 "" V 6230 3650 50  0001 C CNN
F 3 "~" H 6300 3650 50  0001 C CNN
	1    6300 3650
	0    1    1    0   
$EndComp
Wire Wire Line
	6150 3650 5800 3650
$EndSCHEMATC
