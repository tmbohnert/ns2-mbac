;# WEIRD Sencario A§: Montiroing Vulcano Unrest and Erruptions at Hekla Vulcano
;#
;# Thomas Michael Bohnert
;#
;# Version:
;# 0.02, 06/04/2007

# TODO
# Coordinates of the GPS nodes


#####################################################################
# General Parameters
#####################################################################
set opt(title) zero	
set opt(seed) -1	
set opt(stop) 100	;# Stop time.
set opt(scenario) scenA3 ;# Default is the original one. Alternatives are: snBwTest

		
#####################################################################
# Wireless Setup
#####################################################################
set opt(chan)   Channel/WirelessChannel    ; ;# channel type
set opt(prop)   Propagation/TwoRayGround   ; ;# radio-propagation model
set opt(netif)  Phy/WirelessPhy            ; ;# network interface type
set opt(mac)    Mac/802_16            			; ;# MAC type
set opt(ifq)    Queue/DropTail/PriQueue    ; ;# interface queue type
set opt(ll)     LL                         ; ;# link layer type
set opt(ant)    Antenna/OmniAntenna        ; ;# antenna model
set opt(ifqlen) 10                       ; ;# max packet in ifq
set opt(nn)     2        										; ;# number of mobilenodes
set opt(rp)     DSDV     										; ;# routing protocol
set opt(x)      20000            						; ;# X dimension of topography
set opt(y)      20000           							; ;# Y dimension of topography


#####################################################################
# Topology Setup
#####################################################################
set opt(defNumGPSNodes) 5 ;# number of access links for web traffic
set opt(bsX) 1000
set opt(bsY) 19000
set opt(bsZ) 0

#####################################################################
# Traffic Setup
#####################################################################
#set opt(ugsRate) 100000000
set opt(ugsRate) 20000000
set opt(dynRate) 0


#************************************* FUNCTIONS ******************************************
		
proc getOpt {argc argv} {
	global opt genConfig
	for {set i 0} {$i < $argc} {incr i} {
		set arg [lindex $argv $i]
		if {[string range $arg 0 0] != "-"} continue
		set name [string range $arg 1 end]
		set opt($name) [lindex $argv [expr $i+1]]
		puts "opt($name): $opt($name)"
	}
}

proc genConfig {} {
	global opt
	set opt(traceFileName) weird-$opt(scenario).tr
	set opt(namFileName) weird-$opt(scenario).nam
}

proc cfgPhyChannel {} {
#(..)/indep-utils/propagation> ./a.out -m TwoRayGround -fr 3500000000 20000
#distance = 20000
#propagation model: TwoRayGround

#Selected parameters:
#transmit power: 0.281838
#frequency: 3.5e+09
#transmit antenna gain: 1
#receive antenna gain: 1
#system loss: 1
#transmit antenna height: 1.5
#receive antenna height: 1.5
#Receiving threshold RXThresh_ is: 8.91754e-18

Phy/WirelessPhy set Pt_ 0.281838
Phy/WirelessPhy set freq_ 3.5e+09
#Phy/WirelessPhy set L_ 1.0
Phy/WirelessPhy set RXThresh_ 8.91754e-18
Phy/WirelessPhy set CSThresh_ [expr 0.9*[Phy/WirelessPhy set RXThresh_]]

}

proc cfgScenarioA3Default {} {
  global ns opt
  puts "Original WEIRD Scenario A3"

	set topo [new Topography]	   ;#topography object
	$topo load_flatgrid $opt(x) $opt(y)
 
	# god needs to know the number of all wireless interfaces MN+BS
	create-god 6
	set channel [new $opt(chan)]

	$ns node-config	-adhocRouting $opt(rp) \
		-llType $opt(ll) \
    -macType $opt(mac) \
		-ifqType $opt(ifq) \
		-ifqLen $opt(ifqlen) \
		-antType $opt(ant) \
		-propType $opt(prop) \
		-phyType $opt(netif) \
		-channel $channel \
		-topoInstance $topo \
		-wiredRouting OFF \
		-agentTrace ON \
		-routerTrace OFF \
		-macTrace ON \
		-movementTrace OFF

	# Base station (BS)
	set bs_(0) [$ns node]
	$bs_(0) random-motion 0  ;#disable random motion
	$bs_(0) set X_ $opt(bsX)
	$bs_(0) set Y_ $opt(bsY)
	$bs_(0) set Z_ $opt(bsZ)
	#$ns initial_bs_pos $bs_(0) 22
	# Downlink traffic
	set udp_bs [new Agent/UDP]
	$udp_bs set fid_ 0
	$ns attach-agent $bs_(0) $udp_bs
	set cbr_udp_bs [new Application/Traffic/UGS]
	$cbr_udp_bs attach-agent $udp_bs
	$cbr_udp_bs set type_ CBR
	$cbr_udp_bs set packet_size_ 1000;
	$cbr_udp_bs set rate_ $opt(ugsRate) ;
	$cbr_udp_bs set random_ false
	# Uplink traffic
	set null_bs [new Agent/Null]
	$ns attach-agent $bs_(0) $null_bs
	puts "BS configured"

	# Subscriber Stations (GPS)
	set gps_(1) [$ns node]
	$gps_(1) random-motion 0  ;#disable random motion
	$gps_(1) set X_ 800
	$gps_(1) set Y_ 800
	$gps_(1) set Z_ 0.0
	#$ns initial_gps_pos $gps_(1) 20
	# Uplink traffic
	set udp_gps(1) [new Agent/UDP]
	$udp_gps(1) set fid_ 1
	$ns attach-agent $gps_(1) $udp_gps(1)
	set cbr_udp_gps(1) [new Application/Traffic/UGS]
	$cbr_udp_gps(1) attach-agent $udp_gps(1)
	$cbr_udp_gps(1) set type_ CBR
	$cbr_udp_gps(1) set packet_size_ 1000;
	$cbr_udp_gps(1) set rate_ $opt(ugsRate) ;
	$cbr_udp_gps(1) set random_ false
	# Downlink traffic
	set null_gps(1) [new Agent/Null]
	$ns attach-agent $gps_(1) $null_gps(1)
	puts "GPS 1 node configured"

	# Subscriber Stations (GPS)
	set gps_(2) [$ns node]
	$gps_(2) random-motion 0  ;#disable random motion
	$gps_(2) set X_ 600
	$gps_(2) set Y_ 600
	$gps_(2) set Z_ 0.0
	#$ns initial_gps_pos $gps_(1) 20
	# Uplink traffic
	set udp_gps(2) [new Agent/UDP]
	$udp_gps(2) set fid_ 2
	$ns attach-agent $gps_(2) $udp_gps(2)
	set cbr_udp_gps(2) [new Application/Traffic/UGS]
	$cbr_udp_gps(2) attach-agent $udp_gps(2)
	$cbr_udp_gps(2) set type_ CBR
	$cbr_udp_gps(2) set packet_size_ 1000;
	$cbr_udp_gps(2) set rate_ $opt(ugsRate) ;
	$cbr_udp_gps(2) set random_ false
	# Downlink traffic
	set null_gps(2) [new Agent/Null]
	$ns attach-agent $gps_(2) $null_gps(2)
	puts "GPS 2 node configured"	

	# Subscriber Station (GPS)
	set gps_(3) [$ns node]
	$gps_(3) random-motion 0  ;#disable random motion
	$gps_(3) set X_ 400
	$gps_(3) set Y_ 400
	$gps_(3) set Z_ 0.0
	#$ns initial_gps_pos $gps_(1) 20
	# Uplink traffic
	set udp_gps(3) [new Agent/UDP]
	$udp_gps(3) set fid_ 3
	$ns attach-agent $gps_(3) $udp_gps(3)
	set cbr_udp_gps(3) [new Application/Traffic/UGS]
	$cbr_udp_gps(3) attach-agent $udp_gps(3)
	$cbr_udp_gps(3) set type_ CBR
	$cbr_udp_gps(3) set packet_size_ 1000;
	$cbr_udp_gps(3) set rate_ $opt(ugsRate) ;
	$cbr_udp_gps(3) set random_ false
	# Downlink traffic
	set null_gps(3) [new Agent/Null]
	$ns attach-agent $gps_(3) $null_gps(3)
	puts "GPS 3 node configured"

	# # Subscriber Station (GPS)
	set gps_(4) [$ns node]
	$gps_(4) random-motion 0  ;#disable random motion
	$gps_(4) set X_ 200
	$gps_(4) set Y_ 200
	$gps_(4) set Z_ 0.0
	#$ns initial_gps_pos $gps_(1) 20
	# Uplink traffic
	set udp_gps(4) [new Agent/UDP]
	$udp_gps(4) set fid_ 4
	$ns attach-agent $gps_(4) $udp_gps(4)
	set cbr_udp_gps(4) [new Application/Traffic/UGS]
	$cbr_udp_gps(4) attach-agent $udp_gps(4)
	$cbr_udp_gps(4) set type_ CBR
	$cbr_udp_gps(4) set packet_size_ 1000;
	$cbr_udp_gps(4) set rate_ $opt(ugsRate) ;
	$cbr_udp_gps(4) set random_ false
	# Downlink traffic
	set null_gps(4) [new Agent/Null]
	$ns attach-agent $gps_(4) $null_gps(4)
	puts "GPS 4 node configured"
		
	# # Subscriber Station (GPS)
	set gps_(5) [$ns node]
	$gps_(5) random-motion 0  ;#disable random motion
	$gps_(5) set X_ 100
	$gps_(5) set Y_ 100
	$gps_(5) set Z_ 0.0
	#$ns initial_gps_pos $gps_(1) 20
	# Uplink traffic
	set udp_gps(5) [new Agent/UDP]
	$udp_gps(5) set fid_ 5
	$ns attach-agent $gps_(5) $udp_gps(5)
	set cbr_udp_gps(5) [new Application/Traffic/UGS]
	$cbr_udp_gps(5) attach-agent $udp_gps(1)
	$cbr_udp_gps(5) set type_ CBR
	$cbr_udp_gps(5) set packet_size_ 1000;
	$cbr_udp_gps(5) set rate_ $opt(ugsRate);
	$cbr_udp_gps(5) set random_ false
	# Downlink traffic
	set null_gps(5) [new Agent/Null]
	$ns attach-agent $gps_(5) $null_gps(5)
	puts "GPS 5 node configured"
		
	# Uplink traffic
	for {set i 1} {$i <= 5} {incr i} {
		$ns connect $udp_gps($i) $null_bs
	}

	$ns at 10.0 "$cbr_udp_gps(1) start"
	$ns at $opt(stop)-10 "$cbr_udp_gps(1) stop"
	
}

proc cfgSingleNodeBWTestUL {} {
  global ns opt
  puts "Single Node Bandwidth Test"

	set topo [new Topography]	   ;#topography object
	$topo load_flatgrid $opt(x) $opt(y)
 
	# god needs to know the number of all wireless interfaces MN+BS
	create-god 2
	set channel [new $opt(chan)]

	$ns node-config	-adhocRouting $opt(rp) \
		-llType $opt(ll) \
    -macType $opt(mac) \
		-ifqType $opt(ifq) \
		-ifqLen $opt(ifqlen) \
		-antType $opt(ant) \
		-propType $opt(prop) \
		-phyType $opt(netif) \
		-channel $channel \
		-topoInstance $topo \
		-wiredRouting OFF \
		-agentTrace OFF \
		-routerTrace OFF \
		-macTrace ON \
		-movementTrace OFF

	# Base station (BS)
	set bs_(0) [$ns node]
	$bs_(0) random-motion 0  ;#disable random motion
	$bs_(0) set X_ $opt(bsX)
	$bs_(0) set Y_ $opt(bsY)
	$bs_(0) set Z_ $opt(bsZ)
	#$ns initial_bs_pos $bs_(0) 22
	# Downlink traffic
	set udp_bs [new Agent/UDP]
	$udp_bs set fid_ 0
	$ns attach-agent $bs_(0) $udp_bs
	set cbr_udp_bs [new Application/Traffic/UGS]
	$cbr_udp_bs attach-agent $udp_bs
	$cbr_udp_bs set type_ CBR
	$cbr_udp_bs set size_ 1000;
	$cbr_udp_bs set rate_ $opt(ugsRate) ;
	$cbr_udp_bs set random_ false
	# Uplink traffic
	set null_bs [new Agent/Null]
	$ns attach-agent $bs_(0) $null_bs
	puts "BS configured"

	# Subscriber Stations (Temporary Camera Instalation)
	set gps_(1) [$ns node]
	$gps_(1) random-motion 0  ;#disable random motion
	$gps_(1) set X_ 1000
	$gps_(1) set Y_ 4000
	$gps_(1) set Z_ 0.0
	# Uplink traffic
	set udp_gps(1) [new Agent/UDP]
	$udp_gps(1) set fid_ 1
	$ns attach-agent $gps_(1) $udp_gps(1)
	set cbr_udp_gps(1) [new Application/Traffic/UGS]
	$cbr_udp_gps(1) attach-agent $udp_gps(1)
	$cbr_udp_gps(1) set type_ CBR
	$cbr_udp_gps(1) set size_ 200
	$cbr_udp_gps(1) set rate_ $opt(ugsRate)
	$cbr_udp_gps(1) set random_ false
	# Downlink traffic
	set null_gps(1) [new Agent/Null]
	$ns attach-agent $gps_(1) $null_gps(1)
	puts "Temporary camera node configured"

	# Uplink traffic
	$ns connect $udp_gps(1) $null_bs

	# schedule scenario	
	$ns at 10.0 "$cbr_udp_gps(1) start"

# Uplink traffic
	if { $opt(dynRate) == 1 } {
		set n [expr  $opt(stop)/5]
		set cnt 1
		for {set i 0} {$i <= $opt(stop)} {set i [expr $i + $n]} {
			set tr [expr $opt(ugsRate) * $cnt * 4]
			puts "Setting ugsRate at $i to $tr"
			$ns at $i "$cbr_udp_gps(1) set rate_ $tr"
			$ns at $i "puts \"Setting UGS rate at $i to $tr\" "
			incr cnt
		}
	}
	$ns at $opt(stop)-10 "$cbr_udp_gps(1) stop"
}

proc cfgSingleNodeBWTestDL {} {
  global ns opt
  puts "Single Node Bandwidth Test"

	set topo [new Topography]	   ;#topography object
	$topo load_flatgrid $opt(x) $opt(y)
 
	# god needs to know the number of all wireless interfaces MN+BS
	create-god 2
	set channel [new $opt(chan)]

	$ns node-config	-adhocRouting $opt(rp) \
		-llType $opt(ll) \
    -macType $opt(mac) \
		-ifqType $opt(ifq) \
		-ifqLen $opt(ifqlen) \
		-antType $opt(ant) \
		-propType $opt(prop) \
		-phyType $opt(netif) \
		-channel $channel \
		-topoInstance $topo \
		-wiredRouting OFF \
		-agentTrace OFF \
		-routerTrace OFF \
		-macTrace ON \
		-movementTrace OFF

	# Base station (BS)
	set bs_(0) [$ns node]
	$bs_(0) random-motion 0  ;#disable random motion
	$bs_(0) set X_ $opt(bsX)
	$bs_(0) set Y_ $opt(bsY)
	$bs_(0) set Z_ $opt(bsZ)
	#$ns initial_bs_pos $bs_(0) 22
	# Downlink traffic
	set udp_bs [new Agent/UDP]
	$udp_bs set fid_ 0
	$ns attach-agent $bs_(0) $udp_bs
	set cbr_udp_bs [new Application/Traffic/UGS]
	$cbr_udp_bs attach-agent $udp_bs
	$cbr_udp_bs set type_ CBR
	$cbr_udp_bs set size_ 1000;
	$cbr_udp_bs set rate_ $opt(ugsRate) ;
	$cbr_udp_bs set random_ false
	# Uplink traffic
	set null_bs [new Agent/Null]
	$ns attach-agent $bs_(0) $null_bs
	puts "BS configured"

	# Subscriber Stations (Temporary Camera Instalation)
	set gps_(1) [$ns node]
	$gps_(1) random-motion 0  ;#disable random motion
	$gps_(1) set X_ 1000
	$gps_(1) set Y_ 4000
	$gps_(1) set Z_ 0.0
	# Uplink traffic
	set udp_gps(1) [new Agent/UDP]
	$udp_gps(1) set fid_ 1
	$ns attach-agent $gps_(1) $udp_gps(1)
	set cbr_udp_gps(1) [new Application/Traffic/UGS]
	$cbr_udp_gps(1) attach-agent $udp_gps(1)
	$cbr_udp_gps(1) set type_ CBR
	$cbr_udp_gps(1) set size_ 200
	$cbr_udp_gps(1) set rate_ $opt(ugsRate)
	$cbr_udp_gps(1) set random_ false
	# Downlink traffic
	set null_gps(1) [new Agent/Null]
	$ns attach-agent $gps_(1) $null_gps(1)
	puts "Temporary camera node configured"

	# Uplink traffic
	$ns connect $udp_gps(1) $null_bs

	# schedule scenario	
	$ns at 10.0 "$cbr_udp_gps(1) start"

# Uplink traffic
	if { $opt(dynRate) == 1 } {
		set n [expr  $opt(stop)/5]
		set cnt 1
		for {set i 0} {$i <= $opt(stop)} {set i [expr $i + $n]} {
			set tr [expr $opt(ugsRate) * $cnt * 4]
			puts "Setting ugsRate at $i to $tr"
			$ns at $i "$cbr_udp_gps(1) set rate_ $tr"
			$ns at $i "puts \"Setting UGS rate at $i to $tr\" "
			incr cnt
		}
	}
	$ns at $opt(stop)-10 "$cbr_udp_gps(1) stop"
}

proc genEventScheduling {} {
	global ns opt
#$ns at $opt(stop) "$ns nam-end-wireless $opt(stop)"
	$ns at $opt(stop) "stop"
	$ns at $opt(stop) "puts \"Simulation End\"; $ns halt"
}

proc stop {} {
  global ns tracefd namtrace opt
  $ns flush-trace
  close $tracefd
#close $namtrace
#after math
	exec gawk -f $opt(scenario)-t_bytes.awk $opt(traceFileName) > $opt(scenario)-t_bytes.dat &
}

#************************************* MAIN ******************************************
exec /usr/bin/clear
puts "====================== Simulation Setup ======================"
# command line options
getOpt $argc $argv
# configuration post-processing
genConfig

Application/Traffic/UGS set rate_ 0
Application/Traffic/UGS set packet_size_ 0
		
set ns				[new Simulator]	; ;#simulatorn object

# Tracing
set tracefd   [open $opt(traceFileName) w]	; ;#trace file
#set namtrace	[open $opt(namFileName) w]	; ;#nam trace file
$ns use-newtrace
$ns trace-all $tracefd
#$ns namtrace-all-wireless $namtrace $opt(x) $opt(y)

# Wireless channel characteristics
cfgPhyChannel


# Create topology
switch $opt(scenario) {
	scenA3 {cfgScenarioA3}
	snBwTestUL {cfgSingleNodeBWTestUL}
	snBwTestDL {cfgSingleNodeBWTestDL}
}

# schedule events
genEventScheduling
puts "====================== Simulation Setup ======================"
$ns run
