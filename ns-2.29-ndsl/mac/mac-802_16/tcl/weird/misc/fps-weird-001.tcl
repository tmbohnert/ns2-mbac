;# WEIRD Simple Fire Prevention Scenario
;#
;# Thomas Michael Bohnert
;#
;# Version:
;# 0.01,

;# Wireless setup
;#===================================
set val(chan)   Channel/WirelessChannel    ; ;# channel type
set val(prop)   Propagation/TwoRayGround   ; ;# radio-propagation model
set val(netif)  Phy/WirelessPhy            ; ;# network interface type
set val(mac)    Mac/802_16            			; ;# MAC type
set val(ifq)    Queue/DropTail/PriQueue    ; ;# interface queue type
set val(ll)     LL                         ; ;# link layer type
set val(ant)    Antenna/OmniAntenna        ; ;# antenna model
set val(ifqlen) 50                         ; ;# max packet in ifq
set val(nn)     4        										; ;# number of mobilenodes
set val(rp)     DSDV     										; ;# routing protocol
set val(x)      1000            						; ;# X dimension of topography
set val(y)      1000           							; ;# Y dimension of topography
set val(stop)   500 												; ;# time of simulation end

set topo [new Topography]	    ; ;#topography object
$topo load_flatgrid $val(x) $val(y)

set chan0 [new $val(chan)]
create-god $val(nn)

;# NS internals
;#===================================
set ns				[new Simulator]	; ;#simulatorn object
set tracefd   [open demo.tr w]	; ;#trace file
set namtrace	[open demo.nam w]	; ;#nam trace file

$ns trace-all $tracefd
$ns namtrace-all-wireless $namtrace $val(x) $val(y)

;# NS mobilenode config
;#===================================
$ns node-config	-adhocRouting $val(rp) \
								-llType $val(ll) \
                -macType $val(mac) \
                -ifqType $val(ifq) \
                -ifqLen $val(ifqlen) \
                -antType $val(ant) \
                -propType $val(prop) \
                -phyType $val(netif) \
                -channel $chan0 \
                -topoInstance $topo \
                -wiredRouting ON \
                -agentTrace OFF \
                -routerTrace OFF \
                -macTrace ON \
                -movementTrace OFF

;#$ns node-config -addressType hierarchical
;#AddrParams set domain_num_ 1	 ;#number of domains
;#lappend cluster_num 1					 ;#number of clusters in each
;#AddrParams set cluster_num_ $cluster_num
;#lappend eilastlevel 4		 			 ;#number of nodes in each cluster
;#set temp {1.0.0 1.0.1 1.0.2 1.0.3}	 ;# hier address to be used for wireless domain


;# Scenario participants
;#===================================
;# Base station (BS)
;#set BS_(0) [$ns node [lindex $temp 0]]
set BS_(0) [$ns node]
$BS_(0) random-motion 0  ;#disable random motion
$BS_(0) set X_ 900
$BS_(0) set Y_ 900
$BS_(0) set Z_ 0.0
;#$ns initial_node_pos $BS_(0) 22

;# Reset to Mobilenode mode
;#$ns node-config	-wiredRouting OFF

;# Subscriber Station (Coordination Center)
;#set node_(1) [$ns node [lindex $temp 1]]
;#$node_(1) base-station [AddrParams addr2id [$BS_(0) node-addr]]
set node_(1) [$ns node]
$node_(1) random-motion 0  ;#disable random motion
$node_(1) set X_ 800
$node_(1) set Y_ 800
$node_(1) set Z_ 0.0
;#$ns initial_node_pos $node_(1) 20

;# Mobile Station (Surveillance Car)
;#set node_(2) [$ns node [lindex $temp 2]]
;#set node_(2) [$ns node]
;#$node_(2) base-station [AddrParams addr2id [$BS_(0) node-addr]]
;#$node_(2) set X_ 850
;#$node_(2) set Y_ 850
;#$node_(2) set Z_ 0.0
;#$ns initial_node_pos $node_(2) 20

;# Mobile Station (Helicopter)
;#set node_(3) [$ns node [lindex $temp 3]]
;#set node_(3) [$ns node]
;#$node_(3) base-station [AddrParams addr2id [$BS_(0) node-addr]]
;#$node_(3) set X_ 110
;#$node_(3) set Y_ 100
;#$node_(3) set Z_ 0.0
;#$ns initial_node_pos $node_(3) 20

;# Traffic
;#===================================
#Base Station
set udp_bs [new Agent/UDP]
$udp_bs set fid_ 0
$ns attach-agent $node_(0) $udp_bs
set null_bs [new Agent/Null]
$ns attach-agent $node_(0) $null_bs
set cbr_udp_bs [new Application/Traffic/UGS]
$cbr_udp_bs attach-agent $udp_bs
$cbr_udp_bs set type_ CBR
$cbr_udp_bs set packet_size_ 1000;
$cbr_udp_bs set rate_ 512Kb ;
$cbr_udp_bs set random_ false


# Subscriber Station
set udp_ss [new Agent/UDP]
$ns attach-agent $node_(1) $udp_ss
$udp_ss set fid_ 1
set null_ss [new Agent/Null]
$ns attach-agent $node_(1) $null_ss
set cbr_udp_ss [new Application/Traffic/UGS]
$cbr_udp_ss attach-agent $udp_ss
$cbr_udp_cc set type_ CBR
$cbr_udp_cc set packet_size_ 1000;
$cbr_udp_cc set rate_ 512Kb ;
$cbr_udp_cc set random_ false

		
$ns at 10.0 "$cbr_udp_ss start"
$ns at 490.0 "$cbr_udp_ss stop"
$ns at 20.0 "$cbr_udp_cc start"
$ns at 490.0 "$cbr_udp_cc stop"
		

;# Mobile Nodes
for {set i 1} {$i < $val(nn)-1 } { incr i } {
    $ns at $val(stop) "$node_($i) reset";
}

;# Nam
$ns at $val(stop) "$ns nam-end-wireless $val(stop)"
$ns at $val(stop) "stop"
$ns at $val(stop) "puts \"end simulation\" ; $ns halt"

;# Stop it!
proc stop {} {
    global ns tracefd namtrace
    $ns flush-trace
    close $tracefd
    close $namtrace
}

$ns run

