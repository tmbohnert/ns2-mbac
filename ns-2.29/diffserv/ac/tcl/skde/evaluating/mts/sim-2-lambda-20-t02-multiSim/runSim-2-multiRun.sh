#!/bin/sh

# Simulationskontroll skript

# !! Some of these paramters have to be set in the actual simulation scripts too! (e.g. skde-href.tcl)
simTime=4100
warmUpTime=500
capacity=2000000;

# voip traffic
for simSet in 1 2 3 4 5 6 7 8 9 10;
do
	for i in 2 3 4 5;
	do
		lossExp="$i"
		
		# reference bandwidth
		mkdir voip-poo-href-$lossExp-$simSet
		cd voip-poo-href-$lossExp-$simSet
		nsttn ../skde-href-2mb.tcl $simTime 0 Voip POO $lossExp
		gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-poo-href-results.txt
		cd ..
		
		mkdir voip-expoo-href-$lossExp-$simSet
		cd voip-expoo-href-$lossExp-$simSet
		nsttn ../skde-href-2mb.tcl $simTime 0 Voip EXPOO $lossExp
		gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-expoo-href-results.txt
		cd ..
	
		# plug-in bandwidth
		mkdir voip-poo-hhat-$lossExp-$simSet
		cd voip-poo-hhat-$lossExp-$simSet
		nsttn  ../skde-hhat-2mb.tcl $simTime 0 Voip POO $lossExp
		gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-poo-hhat-results.txt
		cd ..
	
		mkdir voip-expoo-hhat-$lossExp-$simSet
		cd voip-expoo-hhat-$lossExp-$simSet
		nsttn  ../skde-hhat-2mb.tcl $simTime 0 Voip EXPOO $lossExp
		gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-expoo-hhat-results.txt
		cd ..
	done
done