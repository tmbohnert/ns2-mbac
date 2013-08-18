# Simple awk file printing interarrival times

BEGIN{
# frame length in seconds
fl=0.005
tbytes=0.0;
# t_start and t_finish
ts=1;
tf=0.0;
}

/UGS/{
	if ($1 == "s"){
		printf("%lf %lf\n", $3, $37);
	}
}		
# /UGS/{
# 	if ($1 == "s"){
# 		t=$3;
# 		if(ts == 1){
# 			ts=t;
# 			tf=t;
# 			tbytes+=$37;
# 		}else{
# 			tbytes+=$37;
# 			if("t" != "tf")
# 				printf("%lf %lf\n", t, tbytes/(tf-ts));
# 			tf=t;
# 		}
# 	}
# }