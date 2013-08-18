#--------------------------------------------------------------------------------
# 
# Author: Thomas Bohnert, bothom
# Dates: 07.07.2005
# Notes: 
#
# bothom:mbac
#
#    	 ----
#    | snode0 |-----------
#    	 ----   		10 Mb  	 \
#            				5 ms    \
#					                     \	  ----		   	          -----
#                      					| core |-----------| dnode |
#                     					/	  ----   10 Mb	      -----
#                    				  /        		 5 ms
#    	 ----         				/
#    | snode1 | -----------
#    	----   	10 Mb
#            				5 ms
#
#--------------------------------------------------------------------------------


set ns [new Simulator]
# Trace support
#set nf [open out.nam w]
#$ns namtrace-all $nf
# Simulations times
set simTime 14000
set warmUpTime 2000
# Bottleneck capacity
set bnTxRate [expr 1*pow(2, 20)*8]
set numSrcs 0
# Queue settings
set bufferSize 200 
#(packets)
# DSCP value
set dscp0 00
set dscp1 11
# NAM flow colours. Colours correspond to the fid_ field in the ip_header
# This has nothing to do with classes/dscps! Mapping a colour to a DSCP value and setting fid_ of each flow to its DSCP allows to colour a whole traffic aggregate.
$ns color $dscp0 green 
$ns color $dscp1 blue
# Traffic settings
#set meanFlowIntArrivals 1
#set meanFlowIntArrivals 3.75
set meanFlowIntArrivals 7.5
#set meanFlowIntArrivals 15
#set meanFlowLen 200
set meanFlowLen 300
#set meanFlowLen 600
set packetSize 125B
# Real time (video) streaming traffic
set txRateVideo [expr 256 * pow(2, 10)]
# G.711 PCM codec for voip
set txRateVoip [expr 64 * pow(2, 10)]


puts "Set up the network topology:"
set snode0 [$ns node]
set snode1 [$ns node]
set cr [$ns node]
set dnode [$ns node]
$ns simplex-link $snode0 $cr 100MB 5ms DropTail
$ns simplex-link $snode1 $cr 100MB 5ms DropTail
$ns simplex-link $cr $dnode $bnTxRate 5ms dsRED/edge

#some NAM settings
#$ns duplex-link-op $snode0 $cr orient down-right

puts "Configure the DiffServ queue:"
set dsQueue [[$ns link $cr $dnode] queue]
# physical queues
$dsQueue set numQueues_ 1
# virtual queues
$dsQueue setNumPrec 1
# dscp physical virtual queue mapping
$dsQueue addPHBEntry 0 0 1
# dropper discipline
$dsQueue setMREDMode DROP
# configure the dropper
$dsQueue configQ 0 1 $bufferSize $bufferSize 0.10
# general queue setup
$dsQueue meanPktSize $packetSize
# create policy between sources and destinations
$dsQueue addPolicyEntry [$snode0 id] [$dnode id] Null 0
$dsQueue addPolicyEntry [$snode1 id] [$dnode id] Null 0
# policer downgrade settings	
$dsQueue addPolicerEntry Null 0
# admission control
$dsQueue addAdcEntry $dscp0 SSUM  [expr $bnTxRate] $txRateVideo
$dsQueue enableAdcDebug $dscp0
$dsQueue printAdcSetup

# Schedule events
puts "Setup generators for random variables:"
# (seed = 1 creates pseudo random variables!)
set rgFi [new RNG]
$rgFi seed 1
set rgFl [new RNG]
$rgFl seed 1

# "Call Arrival" follows a Poisson Process interarrival times and flow length are exponentially distributed
puts "Create random variables:"
set rvFi [new RandomVariable/Exponential]
$rvFi set avg_ $meanFlowIntArrivals
$rvFi use-rng $rgFi
set rvFl [new RandomVariable/Exponential]
$rvFl set avg_ $meanFlowLen
$rvFl use-rng $rgFl

puts "Create Pareto ON/OFF (POO) sources:"
set i 1
for {set fStart [$ns now]} {$fStart < $simTime} {set fStart [expr $fStart + [$rvFi value]]} {
	# traffic source
	set udpAgent($i) [new Agent/UDP]
	if {[expr $i % 2] == 0} {
		$ns attach-agent $snode0 $udpAgent($i) 
	} else {
		$ns attach-agent $snode1 $udpAgent($i)
	}
	$udpAgent($i) set prio_ $dscp0
	$udpAgent($i) set fid_ $i
	# traffic sink
	set lossMon($i) [new Agent/LossMonitor]
	$ns attach-agent $dnode $lossMon($i)
	# connect sources
	$ns connect $udpAgent($i) $lossMon($i)
	# attach application
	set poo($i) [new Application/Traffic/Pareto]
	$poo($i) attach-agent $udpAgent($i)
	$poo($i) set packetSize_ $packetSize
	$poo($i) set burst_time_ 500ms
	$poo($i) set idle_time_ 500ms
	$poo($i) set rate_ $txRateVideo
	#$poo($i) set rate_ $txRateVoip
	$poo($i) set shape_ 1.2

	# schedule flow start and end
	$ns at $fStart "$poo($i) start"
	# set stop time of flow
	set flen [$rvFl value]
	if {[expr $fStart + $flen] < $simTime} {
		set fStop [expr $fStart + $flen]
	} else {
		set fStop $simTime
	}
	$ns at $fStop "$poo($i) stop"
	#puts "Source $i schedule: Start: [format "%2.1f" $fStart], Stop: [format "%2.1f" $fStop], flow length: [format "%2.1f" $flen]"
	incr i
}

	# Set queue monitoring
	#set qmon [$ns monitor-queue $cr $dnode [open qmon.out w] 0.1]
	#[$ns link $cr $dnode] queue-sample-timeout

	#for {set i 0} {$i < $simTime} {set i [expr $i + $simTime/1000]} {
		#$ns at $i "puts \"Remaining time [expr $simTime-$i]\""
		#$ns at $i "puts -nonewline \"#\""
		#$ns at [expr $i+0.5]  "flush stdout "
	#}
	 
proc finish {} {
	global ns
	$ns flush-trace
	#close $nf
	exit 0
}

set simTime [expr $simTime + 5.0]
puts "Simulated time  $simTime"
puts "Simulation starts now\n\n"
$ns at $simTime "finish"
$ns run
