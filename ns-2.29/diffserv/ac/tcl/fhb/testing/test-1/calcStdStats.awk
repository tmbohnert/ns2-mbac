#! /bin/awk -f

BEGIN {
	#simTime=ARGV[0];
	#warmUpTime=ARGV[1];
	#capacity=ARGV[2];
	#lossExp=ARGV[3];
	FS=" ";
	#printf("%i %i %i %i\n", simTime, warmUpTime, capacity, lossExp);
	#printf("| Conf pLoss | Real pLoss | Simulation Time | Warmup Time | Total Bits | Link Load |\n");
}{
	if(NR==3){
		ac = $9; #admission control
		ar = $10; #arrived
		r = $11; #sent
		i = $13; #conf ploss
		#printf("\nac=%12f | ar=%12f | r=%12f\n", ac, ar, r);
		
		# received bytes converted to bits
		rBits = r*8;
		# bits received/(simulation time - warmup time)
		rBps = rBits/(simTime-warmUpTime);
		#link load, in percentage
		l = (rBps/capacity)*100;
		#real ploss (or at least what is believed as), convert from Bytes to Bits
		rpl = rBits / ((ar*8) - ((ac*8)*125));
		rpl= 1 - rpl;
		if(rpl < 0)	rpl = 0;
		
		#print the number of bits that have departured the link and the link load
		printf("%12f\t%12f\t%17i\t%13i\t%12.0f\t%11f\n", 10^-lossExp, rpl, simTime, warmUpTime, rBits, l);
	}
}