
//int compDouble(const void *p, const void *s);
/*int acSKDEEstimator::compDouble(const void *p, const void *s){
	if(*(double *)p<*(double *)s)
		return(-1);
	if(*(double *)p==*(double *)s)
		return(0);
	if(*(double *)p>*(double *)s)
		return(1);
	printf("ERROR in compDouble\n");
	return(0);
}*/



void acSKDEEstimator::updateGaussRefEst(int ts, double latest, double oldest){
/*	int tsi=ts-1;
	double add=0.0, sub=0.0;
	
	//mbac:jpoa
	// Bothom: Multitime scale modification missing
		if(latest > latestVol_saver){
			latestVol_saver=latest;
			//printf("\n %lf \t %lf", latest, Scheduler::instance().clock());
		}
	
	//mbac:jpoa
	if(stats_byte[tsi].css==311 && ts==1)
		;
		
	//update the reference bandwidth for a Gaussian Kernel, based on Gaussian assumption
	stats_byte[tsi].hRef=stats_byte[tsi].sigHat*pow(4.0, 0.333)*pow(stats_byte[tsi].css, -0.3333);
	
	//calculate additive and subtractive value
	add=stdNormalKernel((stats_byte[tsi].hRef>0)?((ep.x[tsi]-latest)/stats_byte[tsi].hRef):(LONG_MAX));
	sub=(stats_byte[tsi].css==meter->getSampleSize(ts))?(stdNormalKernel((stats_byte[tsi].hRef>0)?((ep.x[tsi]-oldest)/stats_byte[tsi].hRef):(LONG_MAX))):(0.0);
	
	//update the kernel sum, sum(K((x-Xi)/h)) - add the lates measurement, subtract the replaced one (the oldest)
	stats_byte[tsi].sumRefKernel=stats_byte[tsi].sumRefKernel+add-sub;
	
	if(stats_byte[tsi].hRef>0.0){
		stats_byte[tsi].FxHref=stats_byte[tsi].sumRefKernel/(stats_byte[tsi].css);
	}
	else
		stats_byte[tsi].FxHref=1.0;
	*/
}
	
void acSKDEEstimator::updateStatistics(int ts, int css, double latestVol, double oldestVol){
/*	double oldVol;
	int tsi=ts-1;
	//int n=stats_byte[tsi].n;
	
	if(ts==1 && css==311)
		printf("Hello");
	//update current sample size
	stats_byte[tsi].css=css; 
	
	if(stats_byte[tsi].css<meter->getSampleSize(ts)){//yet no full-size sample captured
		oldVol=0.0;
	}	else{
		oldVol=oldestVol; //preserve the inidicator (-1) if no full-size sample is available
	}
	
	//update the sums
	stats_byte[tsi].sum+=(latestVol-oldVol);
	stats_byte[tsi].sqrSum+=(pow(latestVol, 2)-pow(oldVol, 2));
	
	//jpoa:mbac
	if(stats_byte[tsi].sqrSum <= 0 ) // cannot be zero. It may happen when the flows stop arriving and sqrSum gets decreased
		stats_byte[tsi].sqrSum = 0;
	//jpoa:mbac

	//mean and std deviation
	stats_byte[tsi].myHat=stats_byte[tsi].sum/stats_byte[tsi].css;
	double square=stats_byte[tsi].sqrSum/((stats_byte[tsi].css>1)?(stats_byte[tsi].css-1):(1))-pow(stats_byte[tsi].myHat, 2);
	printf("%lf", square);
	//stats_byte[tsi].sigHat=sqrt(square);
	
	//calculate the estimate based on the reference model (hRef)
	updateGaussRefEst(ts, latestVol, oldestVol);

	*/
}