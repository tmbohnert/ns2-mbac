#! /bin/awk -f

BEGIN { 
	FS=" ";
	printf("| Conf pLoss | Real pLoss | Simulation Time | Warmup Time | Total Bits | Link Load |\n");
}{
	ac = $9; #admission control
	ar = $10; #arrived
	r = $11; #sent
	i = $13; #conf ploss
	#printf("\nac=%12f | ar=%12f | r=%12f\n", ac, ar, r);
	
	# received bytes converted to bits
	rBits = r*8;
	# bits received/(simulation time - warmup time)
	rBps = rBits/500;
	#link load, in percentage
	l = (rBps/1000000)*100;
	
	#real ploss (or at least what is believed as), convert from Bytes to Bits
	rpl = rBits / ((ar*8) - ((ac*8)*125));
	rpl= 1 - rpl;
	if(rpl < 0)	rpl = 0;
	
	#print the number of bits that have departured the link and the link load
	printf("|%12f|%12f|%17i|%13i|%12.0f|%11f|\n",10^-5, rpl, 1000, 500, rBits, l);
}