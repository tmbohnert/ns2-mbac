# This script is created by P.L. Wu <wupl@cse.nsysu.edu.tw>
#===================================
#              �w�q�����ܼ�                           
#===================================

set val(chan)   Channel/WirelessChannel    ;# channel type
set val(prop)   Propagation/TwoRayGround   ;# radio-propagation model
set val(netif)  Phy/WirelessPhy            ;# network interface type
set val(mac)    Mac/802_16            ;# MAC type
set val(ifq)    Queue/DropTail/PriQueue    ;# interface queue type
set val(ll)     LL                         ;# link layer type
set val(ant)    Antenna/OmniAntenna        ;# antenna model
set val(ifqlen) 50                         ;# max packet in ifq
set val(nn)     9        ;# number of mobilenodes
set val(rp)     DSDV     ;# routing protocol
set val(x)      1000            ;# X dimension of topography
set val(y)      1000           ;# Y dimension of topography
set val(stop)   10.0 ;# time of simulation end

#===================================
#               �إ߬����ɮ�                         
#===================================

#�]�wtrace file
set ns            [new Simulator]	;#����ns simulator
set tracefd       [open demo.tr w]	;#����trace file
set namtrace       [open demo.nam w]	;#���Ͳ���nam trace file

$ns trace-all $tracefd
$ns namtrace-all-wireless $namtrace $val(x) $val(y)

set topo       [new Topography]	    ;#����topography object
$topo load_flatgrid $val(x) $val(y)

create-god $val(nn)

#�إ�channel
set chan0 [new $val(chan)]

#===================================
#        �]�wMobileNode���Ѽ�                      
#===================================

#�]�wMobileNode���Ѽ�
$ns node-config -adhocRouting $val(rp) \
                -llType $val(ll) \
                -macType $val(mac) \
                -ifqType $val(ifq) \
                -ifqLen $val(ifqlen) \
                -antType $val(ant) \
                -propType $val(prop) \
                -phyType $val(netif) \
                -channel $chan0 \
                -topoInstance $topo \
                -agentTrace OFF \
                -routerTrace OFF \
                -macTrace ON \
                -movementTrace OFF

#===================================
#             ����Node              
#===================================

#�إ߲�0��Node
set node_(0) [$ns node]
$node_(0) set X_ 800
$node_(0) set Y_ 800
$node_(0) set Z_ 0.0
$ns initial_node_pos $node_(0) 20

#�إ߲�1��Node
set node_(1) [$ns node]
$node_(1) set X_ 800
$node_(1) set Y_ 900
$node_(1) set Z_ 0.0
$ns initial_node_pos $node_(1) 20

#�إ߲�2��Node
set node_(2) [$ns node]
$node_(2) set X_ 900
$node_(2) set Y_ 900
$node_(2) set Z_ 0.0
$ns initial_node_pos $node_(2) 20

#�إ߲�3��Node
set node_(3) [$ns node]
$node_(3) set X_ 900
$node_(3) set Y_ 800
$node_(3) set Z_ 0.0
$ns initial_node_pos $node_(3) 20

#�إ߲�4��Node
set node_(4) [$ns node]
$node_(4) set X_ 900
$node_(4) set Y_ 700
$node_(4) set Z_ 0.0
$ns initial_node_pos $node_(4) 20

#�إ߲�5��Node
set node_(5) [$ns node]
$node_(5) set X_ 800
$node_(5) set Y_ 700
$node_(5) set Z_ 0.0
$ns initial_node_pos $node_(5) 20

#�إ߲�6��Node
set node_(6) [$ns node]
$node_(6) set X_ 700
$node_(6) set Y_ 700
$node_(6) set Z_ 0.0
$ns initial_node_pos $node_(6) 20

#�إ߲�7��Node
set node_(7) [$ns node]
$node_(7) set X_ 700
$node_(7) set Y_ 800
$node_(7) set Z_ 0.0
$ns initial_node_pos $node_(7) 20

#�إ߲�8��Node
set node_(8) [$ns node]
$node_(8) set X_ 700
$node_(8) set Y_ 900
$node_(8) set Z_ 0.0
$ns initial_node_pos $node_(8) 20

#===================================
#               �]�w�s�u                                 
#===================================

#�]�w��0�ӳs�u(CBR-UDP)
set udp0 [new Agent/UDP]
$ns attach-agent $node_(1) $udp0
set null0 [new Agent/Null]
$ns attach-agent $node_(0) $null0
$ns connect $udp0 $null0
$udp0 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr0 [new Application/Traffic/UGS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr0 attach-agent $udp0
$cbr0 set type_ CBR
$cbr0 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr0 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr0 set random_ false
$ns at 0.0 "$cbr0 start"
$ns at 10.0 "$cbr0 stop"

#�]�w��1�ӳs�u(CBR-UDP)
set udp1 [new Agent/UDP]
$ns attach-agent $node_(2) $udp1
set null1 [new Agent/Null]
$ns attach-agent $node_(0) $null1
$ns connect $udp1 $null1
$udp1 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr1 [new Application/Traffic/ertPS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr1 attach-agent $udp1
$cbr1 set type_ CBR
$cbr1 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr1 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr1 set random_ false
$ns at 0.0 "$cbr1 start"
$ns at 10.0 "$cbr1 stop"

#�]�w��2�ӳs�u(CBR-UDP)
set udp2 [new Agent/UDP]
$ns attach-agent $node_(3) $udp2
set null2 [new Agent/Null]
$ns attach-agent $node_(0) $null2
$ns connect $udp2 $null2
$udp2 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr2 [new Application/Traffic/rtPS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr2 attach-agent $udp2
$cbr2 set type_ CBR
$cbr2 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr2 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr2 set random_ false
$ns at 0.0 "$cbr2 start"
$ns at 10.0 "$cbr2 stop"

#�]�w��3�ӳs�u(CBR-UDP)
set udp3 [new Agent/UDP]
$ns attach-agent $node_(4) $udp3
set null3 [new Agent/Null]
$ns attach-agent $node_(0) $null3
$ns connect $udp3 $null3
$udp3 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr3 [new Application/Traffic/UGS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr3 attach-agent $udp3
$cbr3 set type_ CBR
$cbr3 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr3 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr3 set random_ false
$ns at 0.0 "$cbr3 start"
$ns at 10.0 "$cbr3 stop"

#�]�w��4�ӳs�u(CBR-UDP)
set udp4 [new Agent/UDP]
$ns attach-agent $node_(5) $udp4
set null4 [new Agent/Null]
$ns attach-agent $node_(0) $null4
$ns connect $udp4 $null4
$udp4 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr4 [new Application/Traffic/ertPS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr4 attach-agent $udp4
$cbr4 set type_ CBR
$cbr4 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr4 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr4 set random_ false
$ns at 0.0 "$cbr4 start"
$ns at 10.0 "$cbr4 stop"

#�]�w��5�ӳs�u(CBR-UDP)
set udp5 [new Agent/UDP]
$ns attach-agent $node_(6) $udp5
set null5 [new Agent/Null]
$ns attach-agent $node_(0) $null5
$ns connect $udp5 $null5
$udp5 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr5 [new Application/Traffic/rtPS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr5 attach-agent $udp5
$cbr5 set type_ CBR
$cbr5 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr5 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr5 set random_ false
$ns at 0.0 "$cbr5 start"
$ns at 10.0 "$cbr5 stop"

#�]�w��6�ӳs�u(CBR-UDP)
set udp6 [new Agent/UDP]
$ns attach-agent $node_(7) $udp6
set null6 [new Agent/Null]
$ns attach-agent $node_(0) $null6
$ns connect $udp6 $null6
$udp6 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr6 [new Application/Traffic/UGS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr6 attach-agent $udp6
$cbr6 set type_ CBR
$cbr6 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr6 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr6 set random_ false
$ns at 0.0 "$cbr6 start"
$ns at 10.0 "$cbr6 stop"

#�]�w��7�ӳs�u(CBR-UDP)
set udp7 [new Agent/UDP]
$ns attach-agent $node_(8) $udp7
set null7 [new Agent/Null]
$ns attach-agent $node_(0) $null7
$ns connect $udp7 $null7
$udp7 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr7 [new Application/Traffic/ertPS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr7 attach-agent $udp7
$cbr7 set type_ CBR
$cbr7 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr7 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr7 set random_ false
$ns at 0.0 "$cbr7 start"
$ns at 10.0 "$cbr7 stop"

#�]�w��8�ӳs�u(CBR-UDP)
set udp8 [new Agent/UDP]
$ns attach-agent $node_(0) $udp8
set null8 [new Agent/Null]
$ns attach-agent $node_(1) $null8
$ns connect $udp8 $null8
$udp8 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr8 [new Application/Traffic/rtPS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr8 attach-agent $udp8
$cbr8 set type_ CBR
$cbr8 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr8 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr8 set random_ false
$ns at 0.0 "$cbr8 start"
$ns at 10.0 "$cbr8 stop"

#�]�w��9�ӳs�u(CBR-UDP)
set udp9 [new Agent/UDP]
$ns attach-agent $node_(0) $udp9
set null9 [new Agent/Null]
$ns attach-agent $node_(2) $null9
$ns connect $udp9 $null9
$udp9 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr9 [new Application/Traffic/UGS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr9 attach-agent $udp9
$cbr9 set type_ CBR
$cbr9 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr9 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr9 set random_ false
$ns at 0.0 "$cbr9 start"
$ns at 10.0 "$cbr9 stop"

#�]�w��10�ӳs�u(CBR-UDP)
set udp10 [new Agent/UDP]
$ns attach-agent $node_(0) $udp10
set null10 [new Agent/Null]
$ns attach-agent $node_(3) $null10
$ns connect $udp10 $null10
$udp10 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr10 [new Application/Traffic/ertPS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr10 attach-agent $udp10
$cbr10 set type_ CBR
$cbr10 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr10 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr10 set random_ false
$ns at 0.0 "$cbr10 start"
$ns at 10.0 "$cbr10 stop"

#�]�w��11�ӳs�u(CBR-UDP)
set udp11 [new Agent/UDP]
$ns attach-agent $node_(0) $udp11
set null11 [new Agent/Null]
$ns attach-agent $node_(4) $null11
$ns connect $udp11 $null11
$udp11 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr11 [new Application/Traffic/rtPS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr11 attach-agent $udp11
$cbr11 set type_ CBR
$cbr11 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr11 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr11 set random_ false
$ns at 0.0 "$cbr11 start"
$ns at 10.0 "$cbr11 stop"

#�]�w��12�ӳs�u(CBR-UDP)
set udp12 [new Agent/UDP]
$ns attach-agent $node_(0) $udp12
set null12 [new Agent/Null]
$ns attach-agent $node_(5) $null12
$ns connect $udp12 $null12
$udp12 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr12 [new Application/Traffic/UGS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr12 attach-agent $udp12
$cbr12 set type_ CBR
$cbr12 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr12 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr12 set random_ false
$ns at 0.0 "$cbr12 start"
$ns at 10.0 "$cbr12 stop"

#�]�w��13�ӳs�u(CBR-UDP)
set udp13 [new Agent/UDP]
$ns attach-agent $node_(0) $udp13
set null13 [new Agent/Null]
$ns attach-agent $node_(6) $null13
$ns connect $udp13 $null13
$udp13 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr13 [new Application/Traffic/ertPS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr13 attach-agent $udp13
$cbr13 set type_ CBR
$cbr13 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr13 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr13 set random_ false
$ns at 0.0 "$cbr13 start"
$ns at 10.0 "$cbr13 stop"

#�]�w��14�ӳs�u(CBR-UDP)
set udp14 [new Agent/UDP]
$ns attach-agent $node_(0) $udp14
set null14 [new Agent/Null]
$ns attach-agent $node_(7) $null14
$ns connect $udp14 $null14
$udp14 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr14 [new Application/Traffic/rtPS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr14 attach-agent $udp14
$cbr14 set type_ CBR
$cbr14 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr14 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr14 set random_ false
$ns at 0.0 "$cbr14 start"
$ns at 10.0 "$cbr14 stop"

#�]�w��15�ӳs�u(CBR-UDP)
set udp15 [new Agent/UDP]
$ns attach-agent $node_(0) $udp15
set null15 [new Agent/Null]
$ns attach-agent $node_(8) $null15
$ns connect $udp15 $null15
$udp15 set fid_ 2	;#�bNAM���AUDP���s�u�|�H������
set cbr15 [new Application/Traffic/UGS]	;#�bUDP�s�u���W�إ�CBR���ε{��
$cbr15 attach-agent $udp15
$cbr15 set type_ CBR
$cbr15 set packet_size_ 1000;#�]�w�ʥ]�j�p
$cbr15 set rate_ 512Kb ;#�]�w�ǿ�t�v
$cbr15 set random_ false
$ns at 0.0 "$cbr15 start"
$ns at 10.0 "$cbr15 stop"

#===================================
#              ��������                                   
#===================================

#�]�wPing�M�Ϊ�recv function
Agent/Ping instproc recv {from rtt} {
    $self instvar node_
    puts "node [$node_ id] received ping answer from $from with round-trip-time $rtt ms."
}

# �i�DMobileNode�����w����
for {set i 0} {$i < $val(nn) } { incr i } {
    $ns at $val(stop) "$node_($i) reset";
}

# ����nam�P������
$ns at $val(stop) "$ns nam-end-wireless $val(stop)"
$ns at $val(stop) "stop"
$ns at 10.0 "puts \"end simulation\" ; $ns halt"

# �]�w�������Ϊ�stop function
proc stop {} {
    global ns tracefd namtrace
    $ns flush-trace
    close $tracefd
    close $namtrace
}

$ns run