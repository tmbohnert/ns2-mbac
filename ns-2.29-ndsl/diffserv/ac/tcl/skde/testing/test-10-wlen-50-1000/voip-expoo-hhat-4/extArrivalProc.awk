#! /bin/awk -f

BEGIN {
	#wut=ARGV[0]; (warmUpTime)
	#ti=ARGV[1]; (time scale of interest)
	FS=" ";
}{
	if(($2>wut) && ($4==ti)){
		printf("%12f\t%12f\t%i\n", $2, $5, $4);
	}
}