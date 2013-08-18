;# WEIRD Simple Fire Prevention Scenario
;#
;# Thomas Michael Bohnert
;#
;# Version:
;# 0.02, 06/04/2007

;# Wireless setup
;#===================================
set opt(chan)   Channel/WirelessChannel    ; ;# channel type
set opt(prop)   Propagation/TwoRayGround   ; ;# radio-propagation model
set opt(netif)  Phy/WirelessPhy            ; ;# network interface type
set opt(mac)    Mac/802_16            			; ;# MAC type
set opt(ifq)    Queue/DropTail/PriQueue    ; ;# interface queue type
set opt(ll)     LL                         ; ;# link layer type
set opt(ant)    Antenna/OmniAntenna        ; ;# antenna model
set opt(ifqlen) 50                         ; ;# max packet in ifq
set opt(nn)     2        										; ;# number of mobilenodes
set opt(rp)     DSDV     										; ;# routing protocol
set opt(x)      1000            						; ;# X dimension of topography
set opt(y)      1000           							; ;# Y dimension of topography
set opt(stop)   500 												; ;# time of simulation end

set topo [new Topography]	    ; ;#topography object
$topo load_flatgrid $opt(x) $opt(y)

set chan0 [new $opt(chan)]
create-god $opt(nn)

;# NS internals
;#===================================
set ns				[new Simulator]	; ;#simulatorn object
set tracefd   [open demo.tr w]	; ;#trace file
set namtrace	[open demo.nam w]	; ;#nam trace file

$ns use-newtrace
$ns trace-all $tracefd
$ns namtrace-all-wireless $namtrace $opt(x) $opt(y)

;# NS mobilenode config
;#===================================
$ns node-config	-adhocRouting $opt(rp) \
								-llType $opt(ll) \
                -macType $opt(mac) \
                -ifqType $opt(ifq) \
                -ifqLen $opt(ifqlen) \
                -antType $opt(ant) \
                -propType $opt(prop) \
                -phyType $opt(netif) \
                -channel $chan0 \
                -topoInstance $topo \
                -wiredRouting OFF \
                -agentTrace OFF \
                -routerTrace OFF \
                -macTrace ON \
                -movementTrace OFF

;# Scenario participants
;#===================================
;# Base station (BS)
set BS_(0) [$ns node]
$BS_(0) random-motion 0  ;#disable random motion
$BS_(0) set X_ 900
$BS_(0) set Y_ 900
$BS_(0) set Z_ 0.0
;#$ns initial_BS_pos $BS_(0) 22


		
;# Subscriber Station (Coordination Center)
set gps_(1) [$ns node]
$SS_(1) random-motion 0  ;#disable random motion
$SS_(1) set X_ 800
$SS_(1) set Y_ 800
$SS_(1) set Z_ 0.0
;#$ns initial_SS_pos $SS_(1) 20

;# Subscriber Station (Coordination Center)
set gps_(2) [$ns node]
$SS_(2) random-motion 0  ;#disable random motion
$SS_(2) set X_ 800
$SS_(2) set Y_ 800
$SS_(2) set Z_ 0.0
;#$ns initial_SS_pos $SS_(1) 20
		

;# Traffic
;#===================================
# Base Station
# Downlink
set udp_bs [new Agent/UDP]
$udp_bs set fid_ 0
$ns attach-agent $BS_(0) $udp_bs
set cbr_udp_bs [new Application/Traffic/UGS]
$cbr_udp_bs attach-agent $udp_bs
$cbr_udp_bs set type_ CBR
$cbr_udp_bs set packet_size_ 1000;
$cbr_udp_bs set rate_ 512Kb ;
$cbr_udp_bs set random_ false
# Uplink
set null_bs [new Agent/Null]
$ns attach-agent $BS_(0) $null_bs

# Subscriber Station 0
# Uplink
set udp_ss(0) [new Agent/UDP]
$udp_ss(0) set fid_ 1
$ns attach-agent $SS_(1) $udp_ss(0)
set cbr_udp_ss(0) [new Application/Traffic/UGS]
$cbr_udp_ss(0) attach-agent $udp_ss(0)
$cbr_udp_ss(0) set type_ CBR
$cbr_udp_ss(0) set packet_size_ 1000;
$cbr_udp_ss(0) set rate_ 512Kb ;
$cbr_udp_ss(0) set random_ false
# Downlink
set null_ss(0) [new Agent/Null]
$ns attach-agent $SS_(1) $null_ss(0)

# Subscriber Station 1
# Uplink
set udp_ss(1) [new Agent/UDP]
$udp_ss(1) set fid_ 2
$ns attach-agent $SS_(2) $udp_ss(1)
set cbr_udp_ss(1) [new Application/Traffic/UGS]
$cbr_udp_ss(1) attach-agent $udp_ss(1)
$cbr_udp_ss(1) set type_ CBR
$cbr_udp_ss(1) set packet_size_ 1000;
$cbr_udp_ss(1) set rate_ 512Kb ;
$cbr_udp_ss(1) set random_ false
# Downlink
set null_ss(1) [new Agent/Null]
$ns attach-agent $SS_(2) $null_ss(1)

$ns connect $udp_ss(1) $null_bs
		

;# Event scheduling
;#===================================
$ns at 10.0 "$cbr_udp_ss(1) start"
$ns at $opt(stop)-10 "$cbr_udp_ss(1) stop"
#$ns at 20.0 "$cbr_udp_cc start"
#$ns at 490.0 "$cbr_udp_cc stop"
		

;# Nam
;#===================================
$ns at $opt(stop) "$ns nam-end-wireless $opt(stop)"
$ns at $opt(stop) "stop"
$ns at $opt(stop) "puts \"end simulation\" ; $ns halt"

;# Stop it!
proc stop {} {
    global ns tracefd namtrace
    $ns flush-trace
    close $tracefd
    close $namtrace
}

$ns run

