#!/bin/sh

# Simulationskontroll skript

# !! Some of these paramters have to be set in the actual simulation scripts too! (e.g. skde-href.tcl)
simTime=4100
warmUpTime=500
capacity=1000000;

# voip traffic
for i in 2 3 4 5;
#for i in 4;
do
 	lossExp="$i"
	
	# reference bandwidth
	mkdir voip-poo-ts-02s-1s-$lossExp
	cd voip-poo-ts-02s-1s-$lossExp
	nsttn ../fhb-2mb-wlen-50-ts-02s-1s.tcl $simTime 0 Voip POO $lossExp
	gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-poo-ts-02s-1s-results.txt
	cd ..
	
# 	mkdir voip-expoo-$lossExp
# 	cd voip-expoo-$lossExp
# 	nsttn ../fhb-1mb-wlen-50.tcl $simTime 0 Voip EXPOO $lossExp
# 	gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-expoo-results.txt
# 	cd ..
# 
# 	# plug-in bandwidth
# 	mkdir voip-$lossExp
# 	cd voip-poo-ts-02s-1s-hhat-$lossExp
# 	nsttn  ../skde-hhat-5mb.tcl $simTime 0 Voip POO $lossExp
# 	gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-poo-ts-02s-1s-results.txt
# 	cd ..
# 
# 	mkdir voip-expoo-$lossExp
# 	cd voip-expoo-$lossExp
# 	nsttn  ../skde-hhat-5mb.tcl $simTime 0 Voip EXPOO $lossExp
# 	gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-expoo-results.txt
# 	cd ..
done