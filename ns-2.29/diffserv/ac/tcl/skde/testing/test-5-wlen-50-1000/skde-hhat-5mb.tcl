# Author: Thomas Bohnert
# bothom:mbac
# Date: 22.07.2006

# Comments
# On/Off times are taken from [MarFou03] "Assessing the Quality of Voice Communications Over
# Internet Backbones", IEEE/ACM TRANSACTIONS ON NETWORKING, VOL. 11, NO. 5, OCTOBER 2003
# Flow lenth also from [MarFou03] but as sort of mean between residential (10min) and business (3.5min) users
# Generally, G711 codec with silence supression.


puts "\n\n+++++++ MBAC BASED ON SIMPLE KERNEL DENSITY ESTIMATION +++++++\n"

if { [llength $argv] !=5 } {
	puts "Incorrect format. Please use: \"ns name-of-script.tcl <simTime> <numSources> <appType> <srcModel> <lossExp>\"\n"
	exit 0
} else {
	set simTime [lindex $argv 0]
	set numSources [lindex $argv 1]
	set appType [lindex $argv 2]
	set srcModel [lindex $argv 3]
	set lossExp [lindex $argv 4]
}
puts "Configuration:\n\t simTime: $simTime\n\t numSources: $numSources\n\t appType: $appType\n\t srcModel: $srcModel\n"

set ns [new Simulator]
# Trace support
#set nf [open simTrace.nam w]
#$ns namtrace-all $nf
# Simulations times
#set simTime 5000
set warmUpTime 500

## General settings
# Bottleneck capacity in Mbps and
# buffer size in packets (15ms maximal delay)
# paket size in Bits
set packetSize [expr 125*8]

## 1 Mbit settings
#set bnTxRate [expr 1*pow(10, 6)]
#set bufferSize 15
#set meanFlowIntArrivals 3.75
#set meanFlowLen 300

## 2 Mbit settings
#set bnTxRate [expr 2*pow(10, 6)]
#set bufferSize 30
#set meanFlowIntArrivals 3.75
#set meanFlowLen 300

## 3 Mbit settings
#set bnTxRate [expr 3*pow(10, 6)]
#set bufferSize 45
#set meanFlowIntArrivals 2.5
#set meanFlowLen 300

## 4 Mbit settings
#set bnTxRate [expr 4*pow(10, 6)]
#set bufferSize 60
#set meanFlowIntArrivals 2
#set meanFlowLen 300

## 5 Mbit settings
set bnTxRate [expr 5*pow(10, 6)]
set bufferSize 75
set meanFlowIntArrivals 1.5
set meanFlowLen 300

## 10Mbit settings
#set bnTxRate [expr 10*pow(10, 6)]
#set bufferSize 400
#set meanFlowIntArrivals 0.5
#set meanFlowLen 300

## 30Mbit settings
#set bnTxRate [expr 30*pow(10, 6)]
#set bufferSize 450
#set meanFlowIntArrivals 0.25
#set meanFlowLen 600

## Real time video streaming traffic
set txRateVideo [expr 256 * pow(10, 3)]
set pLossVideo [expr 1*pow(10, -$lossExp)]

## G.711 PCM codec for voip
set txRateVoip [expr 64 * pow(10, 3)]
set pLossVoip [expr 1*pow(10, -$lossExp)]

if { $appType == "Voip" } {
	set txRate $txRateVoip
	set pLoss $pLossVoip
} elseif { $appType == "Video" } {
	set txRate $txRateVideo
	set pLoss $pLossVideo
} else {
	puts "Error: Could not set appType to : $appType\n"
	exit
}	

# DSCP value
set dscp0 00
# NAM flow colours. Colours correspond to the fid_ field in the ip_header
# This has nothing to do with classes/dscps! Mapping a colour to a DSCP value and setting fid_ of each flow to its DSCP allows to colour a whole traffic aggregate.
$ns color $dscp0 green

puts "NETWORK TOPOLOGY SETUP"
set snode0 [$ns node]
set snode1 [$ns node]
set cr [$ns node]
set dnode [$ns node]
$ns simplex-link $snode0 $cr 100MB 5ms DropTail
$ns simplex-link $snode1 $cr 100MB 5ms DropTail
$ns simplex-link $cr $dnode $bnTxRate 5ms dsRED/edge

#some NAM settings
#$ns duplex-link-op $snode0 $cr orient down-right

puts "DIFFSERV QUEUE CONFIGURATION"
set dsQueue [[$ns link $cr $dnode] queue]
# physical queues
$dsQueue set numQueues_ 1
# virtual queues
$dsQueue setNumPrec 1
# dscp physical virtual queue mapping
$dsQueue addPHBEntry $dscp0 0 1
# dropper discipline
$dsQueue setMREDMode DROP
# configure the dropper
$dsQueue configQ 0 1 $bufferSize 1 1 1
# general queue setup
$dsQueue meanPktSize [expr $packetSize/8]
# create policy between sources and destinations
$dsQueue addPolicyEntry [$snode0 id] [$dnode id] Null 0
$dsQueue addPolicyEntry [$snode1 id] [$dnode id] Null 0
# policer downgrade settings	
$dsQueue addPolicerEntry Null 0
# admission control
set hRef 0.0
set j 4
set tau 0.02
set tsm 5
set wlen 50
$dsQueue addAdcEntry $dscp0 SKDE ACCEPT $bnTxRate [expr $bufferSize*$packetSize] $txRate $pLoss TRE $hRef $j IEDE TRE $tau $tsm $wlen TRE
#$dsQueue addAdcEntry $dscp0 SSUM  [expr $bnTxRate] $txRateVideo
$dsQueue printAdcSetup

# Schedule events
puts "GENERATE RANDOM VARIABLES"
# (seed = 1 creates pseudo random variables!)
set rgFi [new RNG]
$rgFi seed 1
set rgFl [new RNG]
$rgFl seed 1

# "Call Arrival" follows a Poisson Process interarrival times and flow length are exponentially distributed
set rvFi [new RandomVariable/Exponential]
$rvFi set avg_ $meanFlowIntArrivals
$rvFi use-rng $rgFi
set rvFl [new RandomVariable/Exponential]
$rvFl set avg_ $meanFlowLen
$rvFl use-rng $rgFl

puts "GENERATE SOURCES"
if { $numSources == 0 } {
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
		# fid_ used for new flow detection for AC
		$udpAgent($i) set fid_ $i

		# traffic sink
		set lossMon($i) [new Agent/LossMonitor]
		$ns attach-agent $dnode $lossMon($i)

		# connect sources
		$ns connect $udpAgent($i) $lossMon($i)
		
		# Application type
		if { $srcModel == "CBR"} {
			set app($i) [new Application/Traffic/CBR]
			$app($i) attach-agent $udpAgent($i)
			$app($i) set packetSize_ [expr $packetSize/8]
			$app($i) set rate_ $txRate
		}
		if { $srcModel == "EXPOO"} {
			set app($i) [new Application/Traffic/Exponential]
			$app($i) attach-agent $udpAgent($i)
			$app($i) set packetSize_ [expr $packetSize/8]
			$app($i) set burst_time_ 300ms
			$app($i) set idle_time_ 600ms
			$app($i) set rate_ $txRate
		}
		if { $srcModel == "POO"} {
			set app($i) [new Application/Traffic/Pareto]
			$app($i) attach-agent $udpAgent($i)
			$app($i) set packetSize_ [expr $packetSize/8]
			$app($i) set burst_time_ 300ms
			$app($i) set idle_time_ 600ms
			$app($i) set rate_ $txRate
			$app($i) set shape_ 1.2
		}

		# schedule flow start and end
		$ns at $fStart "$app($i) start"
		# set stop time of flow
		set flen [$rvFl value]
		if {[expr $fStart + $flen] < $simTime} {
			set fStop [expr $fStart + $flen]
		} else {
			set fStop $simTime
		}
		$ns at $fStop "$app($i) stop"
		incr i
	}
} else {
	set fStart [$ns now]
	for {set i 1 } {$i <= $numSources} { incr i} {
		# traffic source
		set udpAgent($i) [new Agent/UDP]
		if {[expr $i % 2] == 0} {
			$ns attach-agent $snode0 $udpAgent($i) 
		} else {
			$ns attach-agent $snode1 $udpAgent($i)
		}
		$udpAgent($i) set prio_ $dscp0
		# fid_ used for new flow detection for AC
		$udpAgent($i) set fid_ $i

		# traffic sink
		set lossMon($i) [new Agent/LossMonitor]
		$ns attach-agent $dnode $lossMon($i)

		# connect sources
		$ns connect $udpAgent($i) $lossMon($i)

		# Application type
		if { $srcModel == "CBR"} {
			set app($i) [new Application/Traffic/CBR]
			$app($i) attach-agent $udpAgent($i)
			$app($i) set packetSize_ [expr $packetSize/8]
			$app($i) set rate_ $txRate
		}
		if { $srcModel == "EXPOO"} {
			set app($i) [new Application/Traffic/Exponential]
			$app($i) attach-agent $udpAgent($i)
			$app($i) set packetSize_ [expr $packetSize/8]
			$app($i) set burst_time_ 300ms
			$app($i) set idle_time_ 600ms
			$app($i) set rate_ $txRate
		}
		if { $srcModel == "POO"} {
			set app($i) [new Application/Traffic/Pareto]
			$app($i) attach-agent $udpAgent($i)
			$app($i) set packetSize_ [expr $packetSize/8]
			$app($i) set burst_time_ 300ms
			$app($i) set idle_time_ 600ms
			$app($i) set rate_ $txRate
			$app($i) set shape_ 1.2
		}

		# schedule flow start and end
		$ns at $fStart "$app($i) start"
		# set stop time of flow
		set flen [$rvFl value]
		if {[expr $fStart + $flen] < $simTime} {
			set fStop [expr $fStart + $flen]
		} else {
			set fStop $simTime
		}
		$ns at $fStop "$app($i) stop"		
		puts "Source $i schedule: Start: [format "%2.1f" $fStart], Stop: [format "%2.1f" $fStop], flow length: [format "%2.1f" $flen]"
		set fStart [expr $fStart + [$rvFi value]]
	}
}

proc finish {} {
	global ns qmon
	$ns flush-trace
	exit 0
}

proc queue-monitoring {} {
	global ns cr dnode simTime qmon warmUpTime
	puts "CONFIGURE QUEUE MONITORING"
	set qm [$ns monitor-adc-queue $cr $dnode [open qmon w] [expr ($simTime-$warmUpTime-1)/2]]
	[$ns link $cr $dnode] queue-sample-timeout
}

$ns at $warmUpTime "queue-monitoring"
set gameOver [expr $simTime + 5]
$ns at $gameOver "finish"
puts "Simulation runs for $simTime s"
puts "Ready for take off!\n<<<<<<<<<<>>>>>>>>>>>>>>>\n\n"
$ns run
