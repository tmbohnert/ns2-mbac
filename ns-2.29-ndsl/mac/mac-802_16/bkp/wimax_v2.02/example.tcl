# This script is created by P.L. Wu <wupl@cse.nsysu.edu.tw>
#===================================
#              定義模擬變數                           
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
#               建立相關檔案                         
#===================================

#設定trace file
set ns            [new Simulator]	;#產生ns simulator
set tracefd       [open demo.tr w]	;#產生trace file
set namtrace       [open demo.nam w]	;#產生產生nam trace file

$ns trace-all $tracefd
$ns namtrace-all-wireless $namtrace $val(x) $val(y)

set topo       [new Topography]	    ;#產生topography object
$topo load_flatgrid $val(x) $val(y)

create-god $val(nn)

#建立channel
set chan0 [new $val(chan)]

#===================================
#        設定MobileNode的參數                      
#===================================

#設定MobileNode的參數
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
#             產生Node              
#===================================

#建立第0個Node
set node_(0) [$ns node]
$node_(0) set X_ 800
$node_(0) set Y_ 800
$node_(0) set Z_ 0.0
$ns initial_node_pos $node_(0) 20

#建立第1個Node
set node_(1) [$ns node]
$node_(1) set X_ 800
$node_(1) set Y_ 900
$node_(1) set Z_ 0.0
$ns initial_node_pos $node_(1) 20

#建立第2個Node
set node_(2) [$ns node]
$node_(2) set X_ 900
$node_(2) set Y_ 900
$node_(2) set Z_ 0.0
$ns initial_node_pos $node_(2) 20

#建立第3個Node
set node_(3) [$ns node]
$node_(3) set X_ 900
$node_(3) set Y_ 800
$node_(3) set Z_ 0.0
$ns initial_node_pos $node_(3) 20

#建立第4個Node
set node_(4) [$ns node]
$node_(4) set X_ 900
$node_(4) set Y_ 700
$node_(4) set Z_ 0.0
$ns initial_node_pos $node_(4) 20

#建立第5個Node
set node_(5) [$ns node]
$node_(5) set X_ 800
$node_(5) set Y_ 700
$node_(5) set Z_ 0.0
$ns initial_node_pos $node_(5) 20

#建立第6個Node
set node_(6) [$ns node]
$node_(6) set X_ 700
$node_(6) set Y_ 700
$node_(6) set Z_ 0.0
$ns initial_node_pos $node_(6) 20

#建立第7個Node
set node_(7) [$ns node]
$node_(7) set X_ 700
$node_(7) set Y_ 800
$node_(7) set Z_ 0.0
$ns initial_node_pos $node_(7) 20

#建立第8個Node
set node_(8) [$ns node]
$node_(8) set X_ 700
$node_(8) set Y_ 900
$node_(8) set Z_ 0.0
$ns initial_node_pos $node_(8) 20

#===================================
#               設定連線                                 
#===================================

#設定第0個連線(CBR-UDP)
set udp0 [new Agent/UDP]
$ns attach-agent $node_(1) $udp0
set null0 [new Agent/Null]
$ns attach-agent $node_(0) $null0
$ns connect $udp0 $null0
$udp0 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr0 [new Application/Traffic/UGS]	;#在UDP連線之上建立CBR應用程式
$cbr0 attach-agent $udp0
$cbr0 set type_ CBR
$cbr0 set packet_size_ 1000;#設定封包大小
$cbr0 set rate_ 512Kb ;#設定傳輸速率
$cbr0 set random_ false
$ns at 0.0 "$cbr0 start"
$ns at 10.0 "$cbr0 stop"

#設定第1個連線(CBR-UDP)
set udp1 [new Agent/UDP]
$ns attach-agent $node_(2) $udp1
set null1 [new Agent/Null]
$ns attach-agent $node_(0) $null1
$ns connect $udp1 $null1
$udp1 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr1 [new Application/Traffic/ertPS]	;#在UDP連線之上建立CBR應用程式
$cbr1 attach-agent $udp1
$cbr1 set type_ CBR
$cbr1 set packet_size_ 1000;#設定封包大小
$cbr1 set rate_ 512Kb ;#設定傳輸速率
$cbr1 set random_ false
$ns at 0.0 "$cbr1 start"
$ns at 10.0 "$cbr1 stop"

#設定第2個連線(CBR-UDP)
set udp2 [new Agent/UDP]
$ns attach-agent $node_(3) $udp2
set null2 [new Agent/Null]
$ns attach-agent $node_(0) $null2
$ns connect $udp2 $null2
$udp2 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr2 [new Application/Traffic/rtPS]	;#在UDP連線之上建立CBR應用程式
$cbr2 attach-agent $udp2
$cbr2 set type_ CBR
$cbr2 set packet_size_ 1000;#設定封包大小
$cbr2 set rate_ 512Kb ;#設定傳輸速率
$cbr2 set random_ false
$ns at 0.0 "$cbr2 start"
$ns at 10.0 "$cbr2 stop"

#設定第3個連線(CBR-UDP)
set udp3 [new Agent/UDP]
$ns attach-agent $node_(4) $udp3
set null3 [new Agent/Null]
$ns attach-agent $node_(0) $null3
$ns connect $udp3 $null3
$udp3 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr3 [new Application/Traffic/UGS]	;#在UDP連線之上建立CBR應用程式
$cbr3 attach-agent $udp3
$cbr3 set type_ CBR
$cbr3 set packet_size_ 1000;#設定封包大小
$cbr3 set rate_ 512Kb ;#設定傳輸速率
$cbr3 set random_ false
$ns at 0.0 "$cbr3 start"
$ns at 10.0 "$cbr3 stop"

#設定第4個連線(CBR-UDP)
set udp4 [new Agent/UDP]
$ns attach-agent $node_(5) $udp4
set null4 [new Agent/Null]
$ns attach-agent $node_(0) $null4
$ns connect $udp4 $null4
$udp4 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr4 [new Application/Traffic/ertPS]	;#在UDP連線之上建立CBR應用程式
$cbr4 attach-agent $udp4
$cbr4 set type_ CBR
$cbr4 set packet_size_ 1000;#設定封包大小
$cbr4 set rate_ 512Kb ;#設定傳輸速率
$cbr4 set random_ false
$ns at 0.0 "$cbr4 start"
$ns at 10.0 "$cbr4 stop"

#設定第5個連線(CBR-UDP)
set udp5 [new Agent/UDP]
$ns attach-agent $node_(6) $udp5
set null5 [new Agent/Null]
$ns attach-agent $node_(0) $null5
$ns connect $udp5 $null5
$udp5 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr5 [new Application/Traffic/rtPS]	;#在UDP連線之上建立CBR應用程式
$cbr5 attach-agent $udp5
$cbr5 set type_ CBR
$cbr5 set packet_size_ 1000;#設定封包大小
$cbr5 set rate_ 512Kb ;#設定傳輸速率
$cbr5 set random_ false
$ns at 0.0 "$cbr5 start"
$ns at 10.0 "$cbr5 stop"

#設定第6個連線(CBR-UDP)
set udp6 [new Agent/UDP]
$ns attach-agent $node_(7) $udp6
set null6 [new Agent/Null]
$ns attach-agent $node_(0) $null6
$ns connect $udp6 $null6
$udp6 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr6 [new Application/Traffic/UGS]	;#在UDP連線之上建立CBR應用程式
$cbr6 attach-agent $udp6
$cbr6 set type_ CBR
$cbr6 set packet_size_ 1000;#設定封包大小
$cbr6 set rate_ 512Kb ;#設定傳輸速率
$cbr6 set random_ false
$ns at 0.0 "$cbr6 start"
$ns at 10.0 "$cbr6 stop"

#設定第7個連線(CBR-UDP)
set udp7 [new Agent/UDP]
$ns attach-agent $node_(8) $udp7
set null7 [new Agent/Null]
$ns attach-agent $node_(0) $null7
$ns connect $udp7 $null7
$udp7 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr7 [new Application/Traffic/ertPS]	;#在UDP連線之上建立CBR應用程式
$cbr7 attach-agent $udp7
$cbr7 set type_ CBR
$cbr7 set packet_size_ 1000;#設定封包大小
$cbr7 set rate_ 512Kb ;#設定傳輸速率
$cbr7 set random_ false
$ns at 0.0 "$cbr7 start"
$ns at 10.0 "$cbr7 stop"

#設定第8個連線(CBR-UDP)
set udp8 [new Agent/UDP]
$ns attach-agent $node_(0) $udp8
set null8 [new Agent/Null]
$ns attach-agent $node_(1) $null8
$ns connect $udp8 $null8
$udp8 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr8 [new Application/Traffic/rtPS]	;#在UDP連線之上建立CBR應用程式
$cbr8 attach-agent $udp8
$cbr8 set type_ CBR
$cbr8 set packet_size_ 1000;#設定封包大小
$cbr8 set rate_ 512Kb ;#設定傳輸速率
$cbr8 set random_ false
$ns at 0.0 "$cbr8 start"
$ns at 10.0 "$cbr8 stop"

#設定第9個連線(CBR-UDP)
set udp9 [new Agent/UDP]
$ns attach-agent $node_(0) $udp9
set null9 [new Agent/Null]
$ns attach-agent $node_(2) $null9
$ns connect $udp9 $null9
$udp9 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr9 [new Application/Traffic/UGS]	;#在UDP連線之上建立CBR應用程式
$cbr9 attach-agent $udp9
$cbr9 set type_ CBR
$cbr9 set packet_size_ 1000;#設定封包大小
$cbr9 set rate_ 512Kb ;#設定傳輸速率
$cbr9 set random_ false
$ns at 0.0 "$cbr9 start"
$ns at 10.0 "$cbr9 stop"

#設定第10個連線(CBR-UDP)
set udp10 [new Agent/UDP]
$ns attach-agent $node_(0) $udp10
set null10 [new Agent/Null]
$ns attach-agent $node_(3) $null10
$ns connect $udp10 $null10
$udp10 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr10 [new Application/Traffic/ertPS]	;#在UDP連線之上建立CBR應用程式
$cbr10 attach-agent $udp10
$cbr10 set type_ CBR
$cbr10 set packet_size_ 1000;#設定封包大小
$cbr10 set rate_ 512Kb ;#設定傳輸速率
$cbr10 set random_ false
$ns at 0.0 "$cbr10 start"
$ns at 10.0 "$cbr10 stop"

#設定第11個連線(CBR-UDP)
set udp11 [new Agent/UDP]
$ns attach-agent $node_(0) $udp11
set null11 [new Agent/Null]
$ns attach-agent $node_(4) $null11
$ns connect $udp11 $null11
$udp11 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr11 [new Application/Traffic/rtPS]	;#在UDP連線之上建立CBR應用程式
$cbr11 attach-agent $udp11
$cbr11 set type_ CBR
$cbr11 set packet_size_ 1000;#設定封包大小
$cbr11 set rate_ 512Kb ;#設定傳輸速率
$cbr11 set random_ false
$ns at 0.0 "$cbr11 start"
$ns at 10.0 "$cbr11 stop"

#設定第12個連線(CBR-UDP)
set udp12 [new Agent/UDP]
$ns attach-agent $node_(0) $udp12
set null12 [new Agent/Null]
$ns attach-agent $node_(5) $null12
$ns connect $udp12 $null12
$udp12 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr12 [new Application/Traffic/UGS]	;#在UDP連線之上建立CBR應用程式
$cbr12 attach-agent $udp12
$cbr12 set type_ CBR
$cbr12 set packet_size_ 1000;#設定封包大小
$cbr12 set rate_ 512Kb ;#設定傳輸速率
$cbr12 set random_ false
$ns at 0.0 "$cbr12 start"
$ns at 10.0 "$cbr12 stop"

#設定第13個連線(CBR-UDP)
set udp13 [new Agent/UDP]
$ns attach-agent $node_(0) $udp13
set null13 [new Agent/Null]
$ns attach-agent $node_(6) $null13
$ns connect $udp13 $null13
$udp13 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr13 [new Application/Traffic/ertPS]	;#在UDP連線之上建立CBR應用程式
$cbr13 attach-agent $udp13
$cbr13 set type_ CBR
$cbr13 set packet_size_ 1000;#設定封包大小
$cbr13 set rate_ 512Kb ;#設定傳輸速率
$cbr13 set random_ false
$ns at 0.0 "$cbr13 start"
$ns at 10.0 "$cbr13 stop"

#設定第14個連線(CBR-UDP)
set udp14 [new Agent/UDP]
$ns attach-agent $node_(0) $udp14
set null14 [new Agent/Null]
$ns attach-agent $node_(7) $null14
$ns connect $udp14 $null14
$udp14 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr14 [new Application/Traffic/rtPS]	;#在UDP連線之上建立CBR應用程式
$cbr14 attach-agent $udp14
$cbr14 set type_ CBR
$cbr14 set packet_size_ 1000;#設定封包大小
$cbr14 set rate_ 512Kb ;#設定傳輸速率
$cbr14 set random_ false
$ns at 0.0 "$cbr14 start"
$ns at 10.0 "$cbr14 stop"

#設定第15個連線(CBR-UDP)
set udp15 [new Agent/UDP]
$ns attach-agent $node_(0) $udp15
set null15 [new Agent/Null]
$ns attach-agent $node_(8) $null15
$ns connect $udp15 $null15
$udp15 set fid_ 2	;#在NAM中，UDP的連線會以紅色表示
set cbr15 [new Application/Traffic/UGS]	;#在UDP連線之上建立CBR應用程式
$cbr15 attach-agent $udp15
$cbr15 set type_ CBR
$cbr15 set packet_size_ 1000;#設定封包大小
$cbr15 set rate_ 512Kb ;#設定傳輸速率
$cbr15 set random_ false
$ns at 0.0 "$cbr15 start"
$ns at 10.0 "$cbr15 stop"

#===================================
#              結束模擬                                   
#===================================

#設定Ping專用的recv function
Agent/Ping instproc recv {from rtt} {
    $self instvar node_
    puts "node [$node_ id] received ping answer from $from with round-trip-time $rtt ms."
}

# 告訴MobileNode模擬已結束
for {set i 0} {$i < $val(nn) } { incr i } {
    $ns at $val(stop) "$node_($i) reset";
}

# 結束nam與模擬器
$ns at $val(stop) "$ns nam-end-wireless $val(stop)"
$ns at $val(stop) "stop"
$ns at 10.0 "puts \"end simulation\" ; $ns halt"

# 設定模擬器用的stop function
proc stop {} {
    global ns tracefd namtrace
    $ns flush-trace
    close $tracefd
    close $namtrace
}

$ns run