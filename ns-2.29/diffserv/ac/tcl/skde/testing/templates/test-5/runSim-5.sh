#!/bin/sh

# Simulationskontroll skript

# !! Some of these paramters have to be set in the actual simulation scripts too! (e.g. skde-href.tcl)
simTime=4100
warmUpTime=500
capacity=5000000;

# voip traffic
for i in 2;
do
 	lossExp="$i"
	
# 	# reference bandwidth
# 	mkdir voip-poo-href-$lossExp
# 	cd voip-poo-href-$lossExp
# 	sudo nice -n -20 nsttn ../skde-href-5mb.tcl $simTime 0 Voip POO $lossExp
# 	gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-poo-href-results.txt
# 	cd ..
# 	
# 	mkdir voip-expoo-href-$lossExp
# 	cd voip-expoo-href-$lossExp
# 	sudo nice -n -20 nsttn ../skde-href-5mb.tcl $simTime 0 Voip EXPOO $lossExp
# 	gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-expoo-href-results.txt
# 	cd ..

	# plug-in bandwidth
	mkdir voip-poo-hhat-$lossExp
	cd voip-poo-hhat-$lossExp
	sudo nice -n -20 nsttn  ../skde-hhat-5mb.tcl $simTime 0 Voip POO $lossExp
	gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-poo-hhat-results.txt
	cd ..

	mkdir voip-expoo-hhat-$lossExp
	cd voip-expoo-hhat-$lossExp
	sudo nice -n -20 nsttn  ../skde-hhat-5mb.tcl $simTime 0 Voip EXPOO $lossExp
	gawk -v simTime=$simTime -v warmUpTime=$warmUpTime -v capacity=$capacity -v lossExp=$lossExp -f  ../calcStdStats.awk qmon >>  ../voip-expoo-hhat-results.txt
	cd ..
done