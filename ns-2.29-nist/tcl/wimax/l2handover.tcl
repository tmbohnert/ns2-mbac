# Script showing a layer 2 handover in 802.16
# @author rouil
# Scenario: Communication between MN and Sink Node with MN attached to BS.
# Notes:- In order to perform layer 2 handover, the BSs must share the same channel
#       - Additional mechanisms are required in order to continue a communication after a layer 2 handover
#         This is achieved by updating the MN address to simulate layer 3 handovers and traffic redirection.
#         We will provide the code as part of our implementation of IEEE 802.21.
#

#
# Topology scenario:
#
#
#	        |-----|          
#	        | MN0 |                 ; 1.0.1 
#	        |-----|        
#
#
#		  (^)                         (^)
#		   |                           |
#	    |--------------|            |--------------|
#           | Base Station | ; 1.0.0    | Base Station |
#           |--------------|            |--------------|
#	    	   |                           |
#	    	   |                           |
#	     |-----------|                     |
#            | Sink node |---------------------| 		
#            |-----------|  ; 0.0.0
#

#check input parameters
if {$argc != 0} {
	puts ""
	puts "Wrong Number of Arguments! No arguments in this topology"
	puts ""
	exit 
}

# set global variables
set output_dir .
set traffic_start 5
set traffic_stop  15
set simulation_stop 60

#define debug values
Mac/802_16 set debug_ 1
Mac/802_16 set t21_timeout_          20 ;#max 10s

#define coverage area for base station: 20m coverage 
Phy/WirelessPhy set Pt_ 0.025
Phy/WirelessPhy set RXThresh_ 6.12277e-09
Phy/WirelessPhy set CSThresh_ [expr 0.9 *[Phy/WirelessPhy set RXThresh_]]

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

set opt(x)		670			   ;# X dimension of the topography
set opt(y)		670			   ;# Y dimension of the topography

#defines function for flushing and closing files
proc finish {} {
        global ns tf output_dir nb_mn
        $ns flush-trace
        close $tf
	exit 0
}

#create the simulator
set ns [new Simulator]
$ns use-newtrace

#create the topography
set topo [new Topography]
$topo load_flatgrid $opt(x) $opt(y)

#open file for trace
set tf [open $output_dir/out.res w]
$ns trace-all $tf

# set up for hierarchical routing (needed for routing over a basestation)
$ns node-config -addressType hierarchical
AddrParams set domain_num_ 3          			;# domain number
lappend cluster_num 1 1 1           			;# cluster number for each domain 
AddrParams set cluster_num_ $cluster_num
lappend eilastlevel 1 2 2 ;# number of nodes for each cluster 
AddrParams set nodes_num_ $eilastlevel
puts "Configuration of hierarchical addressing done"

# Create God
create-god 3				;# nb_mn + 2 (base station and sink node)

#creates the sink node in first addressing space.
set sinkNode [$ns node 0.0.0]
puts "sink node created"

#create common channel
set channel [new $opt(chan)]

#creates the Access Point (Base station)
$ns node-config -adhocRouting $opt(adhocRouting) \
                 -llType $opt(ll) \
                 -macType $opt(mac) \
                 -ifqType $opt(ifq) \
                 -ifqLen $opt(ifqlen) \
                 -antType $opt(ant) \
                 -propType $opt(prop)    \
                 -phyType $opt(netif) \
                 -channel $channel \
                 -topoInstance $topo \
                 -wiredRouting ON \
                 -agentTrace ON \
                 -routerTrace ON \
                 -macTrace ON  \
                 -movementTrace OFF
#puts "Configuration of base station"

set bstation [$ns node 1.0.0]  
$bstation random-motion 0
#provide some co-ord (fixed) to base station node
$bstation set X_ 50.0
$bstation set Y_ 50.0
$bstation set Z_ 0.0
set clas [new SDUClassifier/Dest]
[$bstation set mac_(0)] add-classifier $clas
#set the scheduler for the node. 
set bs_sched [new WimaxScheduler/BS]
[$bstation set mac_(0)] set-scheduler $bs_sched
[$bstation set mac_(0)] set-channel 0
puts "Base Station 1 created"

set bstation2 [$ns node 2.0.0]  
$bstation2 random-motion 0
#provide some co-ord (fixed) to base station node
$bstation2 set X_ 65.0
$bstation2 set Y_ 50.0
$bstation2 set Z_ 0.0
set clas2 [new SDUClassifier/Dest]
[$bstation2 set mac_(0)] add-classifier $clas2
#set the scheduler for the node.
set bs_sched2 [new WimaxScheduler/BS]
[$bstation2 set mac_(0)] set-scheduler $bs_sched2
[$bstation2 set mac_(0)] set-channel 1
puts "Base Station 2 created"

# creation of the mobile nodes
$ns node-config -wiredRouting OFF \
                -macTrace ON  				;# Mobile nodes cannot do routing.

set wl_node [$ns node 1.0.1] 	;# create the node with given @.	
$wl_node random-motion 0			;# disable random motion
$wl_node base-station [AddrParams addr2id [$bstation node-addr]] ;#attach mn to basestation
$wl_node set X_ 45.0
$wl_node set Y_ 50.0
$wl_node set Z_ 0.0
$ns at 0.0 "$wl_node setdest 70.0 50.0 2.0"
set clas [new SDUClassifier/Dest]
[$wl_node set mac_(0)] add-classifier $clas
#set the scheduler for the node. Must be changed to -shed [new $opt(sched)]
set ss_sched [new WimaxScheduler/SS]
[$wl_node set mac_(0)] set-scheduler $ss_sched
[$wl_node set mac_(0)] set-channel 0
puts "wireless node created ..."			;# debug info
#$ns at 6.5 "$ss_sched send-scan"

#create source traffic
#Create a UDP agent and attach it to node n0
set udp [new Agent/UDP]
$udp set packetSize_ 1500
$ns attach-agent $wl_node $udp

# Create a CBR traffic source and attach it to udp0
set cbr [new Application/Traffic/CBR]
$cbr set packetSize_ 1000
$cbr set interval_ 0.1
$cbr attach-agent $udp

#create an sink into the sink node

# Create the Null agent to sink traffic
set null [new Agent/Null] 
$ns attach-agent $sinkNode $null

# Attach the 2 agents
$ns connect $udp $null


# create the link between sink node and base station
$ns duplex-link $sinkNode $bstation 100Mb 1ms DropTail

# Traffic scenario: here the all start talking at the same time
$ns at $traffic_start "$cbr start"
$ns at $traffic_stop "$cbr stop"

$ns at $simulation_stop "finish"
puts "Running simulation"
$ns run
puts "Simulation done."
