EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 5 11
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
L Connector_Generic:Conn_01x03 J?
U 1 1 5F95761F
P 6400 3650
AR Path="/5F95761F" Ref="J?"  Part="1" 
AR Path="/5F9534D6/5F95761F" Ref="J10"  Part="1" 
AR Path="/5F95F7DC/5F95761F" Ref="J12"  Part="1" 
AR Path="/5F965BCA/5F95761F" Ref="J14"  Part="1" 
F 0 "J14" H 6318 3325 50  0000 C CNN
F 1 "GPIO" H 6318 3416 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x03" H 6400 3650 50  0001 C CNN
F 3 "~" H 6400 3650 50  0001 C CNN
	1    6400 3650
	1    0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J?
U 1 1 5F957625
P 4900 3650
AR Path="/5F957625" Ref="J?"  Part="1" 
AR Path="/5F9534D6/5F957625" Ref="J9"  Part="1" 
AR Path="/5F95F7DC/5F957625" Ref="J11"  Part="1" 
AR Path="/5F965BCA/5F957625" Ref="J13"  Part="1" 
F 0 "J13" H 4818 3967 50  0000 C CNN
F 1 "Power select" H 4818 3876 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x03" H 4900 3650 50  0001 C CNN
F 3 "~" H 4900 3650 50  0001 C CNN
	1    4900 3650
	-1   0    0    -1  
$EndComp
Wire Wire Line
	6200 3650 5750 3650
Wire Wire Line
	5100 3550 5400 3550
Wire Wire Line
	5400 3550 5400 3450
Wire Wire Line
	5100 3750 5600 3750
Wire Wire Line
	5600 3750 5600 3450
$Comp
L power:+3V3 #PWR?
U 1 1 5F957630
P 5400 3450
AR Path="/5F957630" Ref="#PWR?"  Part="1" 
AR Path="/5F9534D6/5F957630" Ref="#PWR049"  Part="1" 
AR Path="/5F95F7DC/5F957630" Ref="#PWR053"  Part="1" 
AR Path="/5F965BCA/5F957630" Ref="#PWR057"  Part="1" 
F 0 "#PWR057" H 5400 3300 50  0001 C CNN
F 1 "+3V3" H 5415 3623 50  0000 C CNN
F 2 "" H 5400 3450 50  0001 C CNN
F 3 "" H 5400 3450 50  0001 C CNN
	1    5400 3450
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 5F957636
P 5600 3450
AR Path="/5F957636" Ref="#PWR?"  Part="1" 
AR Path="/5F9534D6/5F957636" Ref="#PWR050"  Part="1" 
AR Path="/5F95F7DC/5F957636" Ref="#PWR054"  Part="1" 
AR Path="/5F965BCA/5F957636" Ref="#PWR058"  Part="1" 
F 0 "#PWR058" H 5600 3300 50  0001 C CNN
F 1 "+5V" H 5615 3623 50  0000 C CNN
F 2 "" H 5600 3450 50  0001 C CNN
F 3 "" H 5600 3450 50  0001 C CNN
	1    5600 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 3750 6100 3750
Wire Wire Line
	6100 3750 6100 3850
$Comp
L power:GND #PWR?
U 1 1 5F957640
P 6100 3850
AR Path="/5F957640" Ref="#PWR?"  Part="1" 
AR Path="/5F9534D6/5F957640" Ref="#PWR051"  Part="1" 
AR Path="/5F95F7DC/5F957640" Ref="#PWR055"  Part="1" 
AR Path="/5F965BCA/5F957640" Ref="#PWR059"  Part="1" 
F 0 "#PWR059" H 6100 3600 50  0001 C CNN
F 1 "GND" H 6105 3677 50  0000 C CNN
F 2 "" H 6100 3850 50  0001 C CNN
F 3 "" H 6100 3850 50  0001 C CNN
	1    6100 3850
	1    0    0    -1  
$EndComp
Text HLabel 6200 3550 0    50   BiDi ~ 0
GPIO
$Comp
L Device:C C14
U 1 1 5F96CA2C
P 5750 3800
AR Path="/5F9534D6/5F96CA2C" Ref="C14"  Part="1" 
AR Path="/5F95F7DC/5F96CA2C" Ref="C15"  Part="1" 
AR Path="/5F965BCA/5F96CA2C" Ref="C16"  Part="1" 
F 0 "C16" H 5865 3846 50  0000 L CNN
F 1 "100N" H 5865 3755 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 5788 3650 50  0001 C CNN
F 3 "~" H 5750 3800 50  0001 C CNN
	1    5750 3800
	1    0    0    -1  
$EndComp
Connection ~ 5750 3650
Wire Wire Line
	5750 3650 5100 3650
$Comp
L power:GND #PWR?
U 1 1 5F96CB56
P 5750 3950
AR Path="/5F96CB56" Ref="#PWR?"  Part="1" 
AR Path="/5F9534D6/5F96CB56" Ref="#PWR052"  Part="1" 
AR Path="/5F95F7DC/5F96CB56" Ref="#PWR056"  Part="1" 
AR Path="/5F965BCA/5F96CB56" Ref="#PWR060"  Part="1" 
F 0 "#PWR060" H 5750 3700 50  0001 C CNN
F 1 "GND" H 5755 3777 50  0000 C CNN
F 2 "" H 5750 3950 50  0001 C CNN
F 3 "" H 5750 3950 50  0001 C CNN
	1    5750 3950
	1    0    0    -1  
$EndComp
$EndSCHEMATC
