# WEIRD IEEE 802.16 testbed, Serra de Lousa, Portugal.
# @author Thomas Michael Bohnert
# Scenario: Forest Fire Prevention
# 

#check input parameters
if {$argc != 0} {
	puts ""
	puts "Wrong Number of Arguments! No arguments in this topology"
	puts ""
	exit (1)
}

# set global variables
set packet_size	1500			;# packet size in bytes at CBR applications
set gap_size 0.1 ;#compute gap size between packets
puts "gap size=$gap_size"
set simulation_stop 150

#define debug values
Mac/802_16 set debug_ 1

#define coverage area for base station
# indep-utils/propagation: TwoRayGround 1000,
Phy/WirelessPhy set Pt_ 0.281838
Phy/WirelessPhy set RXThresh_ 1.42681e-16 ;#100000m (10km) radius
Phy/WirelessPhy set CSThresh_ [expr 0.9*[Phy/WirelessPhy set RXThresh_]]

# Parameter for wireless nodes
set opt(chan)           Channel/WirelessChannel    ;# channel type
set opt(prop)           Propagation/TwoRayGround   ;# radio-propagation model
set opt(netif)          Phy/WirelessPhy/OFDM       ;# network interface type
set opt(mac)            Mac/802_16                 ;# MAC type
set opt(ifq)            Queue/DropTail/PriQueue    ;# interface queue type
set opt(ll)             LL                         ;# link layer type
set opt(ant)            Antenna/OmniAntenna        ;# antenna model
set opt(ifqlen)         50              	   ;# max packet in ifq
set opt(adhocRouting)   DSDV                       ;# routing protocol

set opt(x)		1200			   ;# X dimension of the topography
set opt(y)		1200			   ;# Y dimension of the topography


######################################################
#create the simulator
set ns [new Simulator]
$ns use-newtrace

#open file for trace
set tf [open out.res w]
$ns trace-all $tf
set wlannamtrace [open nam-wireless-weird-ffp.nam w]
$ns namtrace-all-wireless $wlannamtrace $opt(x) $opt(y)
#set allnamtrace [open nam-all-weird-ffp.nam w]
#$ns namtrace-all $allnamtrace
#$ns nam-end-wireless $simulation_stop


#create the topography
set topo [new Topography]
$topo load_flatgrid $opt(x) $opt(y)

######################################################
# set up for hierarchical routing (needed for routing over a basestation)
$ns node-config -addressType hierarchical
AddrParams set domain_num_ 1 ;# domain number
lappend cluster_num 1 ;# cluster number for each domain
AddrParams set cluster_num_ $cluster_num
lappend eilastlevel 4	;# number of nodes for each cluster (1 for sink and one for mobile nodes + base station
AddrParams set nodes_num_ $eilastlevel
puts "Configuration of hierarchical addressing done"
# Create God
create-god 5			
#puts "God node created"

######################################################
#creates the Access Point (Base station)
$ns node-config -adhocRouting $opt(adhocRouting) \
                 -llType $opt(ll) \
                 -macType $opt(mac) \
                 -ifqType $opt(ifq) \
                 -ifqLen $opt(ifqlen) \
                 -antType $opt(ant) \
                 -propType $opt(prop)    \
                 -phyType $opt(netif) \
                 -channel [new $opt(chan)] \
                 -topoInstance $topo \
                 -wiredRouting ON \
                 -agentTrace ON \
                 -routerTrace ON \
                 -macTrace ON  \
                 -movementTrace ON

set bstation [$ns node 1.0.0]
$bstation random-motion 0
$bstation set X_ 900.0
$bstation set Y_ 900.0
$bstation set Z_ 0.0
set clas_bs [new SDUClassifier/Dest]
[$bstation set mac_(0)] add-classifier $clas_bs
set bs_sched [new WimaxScheduler/BS]
[$bstation set mac_(0)] set-scheduler $bs_sched
[$bstation set mac_(0)] set-channel 0
$bstation label BSTATION
$bstation shape circle
$bstation color blue

######################################################
# creation of the mobilne nodes
$ns node-config -wiredRouting OFF \
                -macTrace ON
                
# coordinations center subscriber station
set ccenter [$ns node 1.0.1]
$ccenter random-motion 0			;# disable random motion
$ccenter base-station [AddrParams addr2id [$bstation node-addr]] ;#attach mn to basestation
$ccenter set X_ 300.0
$ccenter set Y_ 300.0
set clas_cc [new SDUClassifier/Dest]
[$ccenter set mac_(0)] add-classifier $clas_cc
set cc_sched [new WimaxScheduler/SS]
[$ccenter set mac_(0)] set-scheduler $cc_sched
[$ccenter set mac_(0)] set-channel 0
#ccenter, Null agent to sink traffic
set null_cc [new Agent/Null]
$ns attach-agent $ccenter $null_cc
#create source traffic
set udp_cc [new Agent/UDP]
$udp_cc set packetSize_ 1500
$ns attach-agent $ccenter $udp_cc
# Create a CBR traffic source and attach it
set cbr_cc [new Application/Traffic/CBR]
$cbr_cc set packetSize_ $packet_size
$cbr_cc set interval_ $gap_size
$cbr_cc attach-agent $udp_cc
$ccenter label CCENTER
$ccenter shape box
$ccenter color blue
$ccenter label-color blue

# surveilance car mobile node
set scar [$ns node 1.0.2]
$scar color green
$scar random-motion 0
$scar base-station [AddrParams addr2id [$bstation node-addr]] ;#attach mn to basestation
$scar set X_ 700.0
$scar set Y_ 900.0
set clas_sc [new SDUClassifier/Dest]
[$scar set mac_(0)] add-classifier $clas_sc
set sc_sched [new WimaxScheduler/SS]
[$scar set mac_(0)] set-scheduler $sc_sched
[$scar set mac_(0)] set-channel 0
#scar, Null agent to sink traffic
set null_sc [new Agent/Null]
$ns attach-agent $scar $null_sc
#create source traffic
set udp_sc [new Agent/UDP]
$udp_sc set packetSize_ 1500
$ns attach-agent $scar $udp_sc
# Create a CBR traffic source and attach it
set cbr_sc [new Application/Traffic/CBR]
$cbr_sc set packetSize_ $packet_size
$cbr_sc set interval_ $gap_size
$cbr_sc attach-agent $udp_sc
$scar label SCAR
$scar label-color green
#$scar shape box

# helicopter mobile node
set hcopter [$ns node 1.0.3]
$hcopter random-motion 0			;# disable random motion
$hcopter base-station [AddrParams addr2id [$bstation node-addr]] ;#attach mn to basestation
$hcopter set X_ 350.0
$hcopter set Y_ 250.0
set clas_hc [new SDUClassifier/Dest]
[$hcopter set mac_(0)] add-classifier $clas_hc
set hc_sched [new WimaxScheduler/SS]
[$hcopter set mac_(0)] set-scheduler $hc_sched
[$hcopter set mac_(0)] set-channel 0
#s, Null agent to sink traffic
set null_hc [new Agent/Null]
$ns attach-agent $hcopter $null_hc
#create source traffic
set udp_hc [new Agent/UDP]
$udp_hc set packetSize_ 1500
$ns attach-agent $hcopter $udp_hc
# Create a CBR traffic source and attach it
set cbr_hc [new Application/Traffic/CBR]
$cbr_hc set packetSize_ $packet_size
$cbr_hc set interval_ $gap_size
$cbr_hc attach-agent $udp_hc
$hcopter label HCOPTER
$hcopter label-color green
#$scar shape hexagon

#Attach the agents
$ns connect $udp_sc $null_cc
$ns connect $udp_cc $null_hc
$ns connect $udp_hc $null_cc

######################################################
# helicopter mobile node
set fire [$ns node 1.0.5]
$fire set X_ 700.0
$fire set Y_ 700.0
$fire label FIRE
$fire label-color white
$fire shape hexagon


######################################################
$ns at 4 "$fire label-color red"
$ns at 4.01 "$fire setdest 700.0 800.0 10000.0"

# traffic scenario
# scrar reports a fire
$ns at 5 "$cbr_sc start"
$ns at 5.2 "$ns trace-annotate \"SCAR detects and reports a fire\""
# ccenter communicates with the hcopter to lead the hcopter
$ns at 10 "$cbr_cc start"
$ns at 10.2 "$ns trace-annotate \"CCENTER communicates with the HCOPTER to coordinate the operation\""
# hcopter communicates with the ccenter to coordinate the fire fighters
$ns at 15 "$cbr_hc start"
$ns at 15.2 "$ns trace-annotate \"HCOPTER communicates with the CCENTER to report about the operation progress\""

# movement scenario
# fire at x=7000 y=8000
# position all nodes accordingly (NAM Hack!)
$ns at 0.01 "$fire setdest 700.0 800.0 1000.0"
$ns at 0.01 "$ccenter setdest 300.0 300.0 1000.0"
$ns at 0.01 "$scar setdest 700.0 900.0 1000.0"
$ns at 0.01 "$hcopter setdest 350.0 250.0 1000.0"

# scar retreats to a safe place
$ns at 6.0 "$scar setdest  300.0 900.0 10.0"
$ns at 6.0 "$ns trace-annotate \"SCAR detects a fire and retreats from the danger zone\""
# hcopter enters the area
$ns at 10.0 "$hcopter setdest 800.0 700.0 50.0"
$ns at 10.0 "$ns trace-annotate \"HCOPTER takes off towards the danger zone\""
# hcopter revolves the fire
$ns at 30.0 "$hcopter setdest 810.0 910.0 30.0"
$ns at 30.0 "$ns trace-annotate \"CCENTER orders the HCOPTER to move towards NORTH\""
$ns at 45.0 "$hcopter setdest 610.0 900.0 30.0"
$ns at 45.0 "$ns trace-annotate \"CCENTER orders the HCOPTER to move towards WEST\""
$ns at 60.0 "$hcopter setdest 600.0 710.0 30.0"
$ns at 60.0 "$ns trace-annotate \"CCENTER orders the HCOPTER to move towards SOUTH\""
$ns at 75.0 "$hcopter setdest 810.0 700.0 30.0"
$ns at 75.0 "$ns trace-annotate \"CCENTER orders the HCOPTER to move towards EAST\""
# hcopter returns
$ns at 90.0 "$hcopter setdest 350.0 250.0 50.0"
$ns at 90.0 "$ns trace-annotate \". Operation Accomplished. CCENTER orders the HCOPTER to return\""

$ns at $simulation_stop "finish"

######################################################
proc finish {} {
    global ns tf wlannamtrace allnamtrace
    $ns flush-trace
    close $tf
    close $allnamtrace
    close $wlannamtrace
    $ns halt
}

######################################################
# Run the simulation
puts "Running simulation ..."
$ns run
puts "Simulation done."