#!/bin/sh

# Simulationskontroll skript

# !! Some of these paramters have to be set in the actual simulation scripts too! (e.g. skde-href.tcl)
simTime=4100
warmUpTime=500
capacity=4000000;

# voip traffic
for i in 2 3 4 5;
do
 	lossExp="$i"
	
	# reference bandwidth
	mkdir voip-poo-href-$lossExp
	cd voip-poo-href-$lossExp
	nsttn ../skde-href-4mb.tcl $simTime 0 Voip POO $lossExp
	gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-poo-href-results.txt
	cd ..
	
	mkdir voip-expoo-href-$lossExp
	cd voip-expoo-href-$lossExp
	nsttn ../skde-href-4mb.tcl $simTime 0 Voip EXPOO $lossExp
	gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-expoo-href-results.txt
	cd ..

	# plug-in bandwidth
	mkdir voip-poo-hhat-$lossExp
	cd voip-poo-hhat-$lossExp
	nsttn  ../skde-hhat-4mb.tcl $simTime 0 Voip POO $lossExp
	gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-poo-hhat-results.txt
	cd ..

	mkdir voip-expoo-hhat-$lossExp
	cd voip-expoo-hhat-$lossExp
	nsttn  ../skde-hhat-4mb.tcl $simTime 0 Voip EXPOO $lossExp
	gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-expoo-hhat-results.txt
	cd ..
done