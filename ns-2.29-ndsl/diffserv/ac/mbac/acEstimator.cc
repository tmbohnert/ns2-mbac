//
// C++ Implementation: acSKDEEstimator
//
// Description: bothom:mbac
//
//
// Author: Thomas Bohnert <tbohnert@dei.uc.pt>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "acEstimator.h"
#include "acPolicy.h"
#include "acMeter.h"
#include "acDefinitions.h"
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_statistics_double.h>
#include <math.h>
#include <string.h>
#include <limits.h>

int compDoubleFree(const void *p, const void *s){
	if(*(double *)p<*(double *)s)
		return(-1);
	if(*(double *)p==*(double *)s)
		return(0);
	if(*(double *)p>*(double *)s)
		return(1);
	printf("ERROR in compDouble\n");
	return(0);
}

acEstimator::~acEstimator(){}

acSKDEEstimator::acSKDEEstimator(acPolicy *policy, acParameterSKDE::skdeEstParam ep, acParameterSKDE::skdeMeterParam mp):tcl(Tcl::instance()){
	this->policy=policy;
	this->ep=ep;
	meter=new acMeter(mp, (acEstimator *)this);
	stats_byte=new skdeStats[mp.tsm]; //time scale individual statistics
	for(int tsi=0; tsi<mp.tsm; tsi++) { //tsi=time scale array index
		stats_byte[tsi].init();
	}
	
	if(ep.h!=0.0)
		useHref=true;
	else
		useHref=false;
}

void acSKDEEstimator::printConfig(){
	printf("\tEstimator:\n\t\testType = SKDEEST\n\t\tUse plug-in bandwidth = %s\n\t\tPlug-in steps = %i\n\t\tMonitored thresholds (ep.x[]) = ", (useHref)?("false"):("true"), ep.j);
	for(int tsi=0; tsi<meter->getTsm(); tsi++)
		printf("%.0lf ", ep.x[tsi]);
	printf("\n\t\tInterval Estimate = %s", (ep.intEst)?("Enabled"):("Disabled"));
	printf("\n\t\tTracing = %s\n", (ep.trace)?("Enabled"):("Disabled"));
	meter->printConfig();
}

void acSKDEEstimator::meterUpdate(double bytes){
	meter->meterUpdate(bytes);
}

void acSKDEEstimator::updateStatistics(int ts, int css, double latestVol, double oldestVol){
}

void acSKDEEstimator::updateGaussRefEst(int ts, double latest, double oldest){
}

double acSKDEEstimator::estimate(int ts){
	if(ep.intEst){
		return(intEstimate(ts));
	}else{
		if(useHref)
			return(getFxHref(ts));
		else
			return(getFxHhat(ts));
	}
}

double acSKDEEstimator::intEstimate(int ts){
	int tsi=ts-1;
	double qv[30];
	
	if(useHref){
		stats_byte[tsi].FxHrefRec[stats_byte[tsi].idxHref++]=getFxHref(ts); //get the latest estimate
		if(stats_byte[tsi].idxHref==30) 
			stats_byte[tsi].idxHref=0;
		memcpy(qv, stats_byte[tsi].FxHrefRec, 30*sizeof(double)); //copy it to a working vector to protect the record
	}else{
		stats_byte[tsi].FxHhatRec[stats_byte[tsi].idxHhat++]=getFxHhat(ts);
		if(stats_byte[tsi].idxHhat==30)
			stats_byte[tsi].idxHhat=0;
		memcpy(qv, stats_byte[tsi].FxHhatRec, 30*sizeof(double));
	}
	qsort(qv, 30, sizeof(double), compDoubleFree);
	return(qv[3]);
}

double acSKDEEstimator::getFxHref(int ts){
	double *data=NULL;
	int tsi=ts-1;

	stats_byte[tsi].css=meter->getSample(ts, &data); //get the latest sample
	stats_byte[tsi].sigHat=gsl_stats_sd(data, 1, (int)stats_byte[tsi].css);

	stats_byte[tsi].sumRefKernel=0.0;
	stats_byte[tsi].hRef=stats_byte[tsi].sigHat*pow(4.0, 0.333)*pow(stats_byte[tsi].css, -0.3333);
	for(int idx=0; idx<stats_byte[tsi].css; idx++) //TODO: optimize (Herrmann skript)
		stats_byte[tsi].sumRefKernel+=stdNormalKernel(((ep.x[tsi]-data[idx])/stats_byte[tsi].hRef));

	delete []data;
	stats_byte[tsi].FxHref=stats_byte[tsi].sumRefKernel/(stats_byte[tsi].css);
	if(ep.trace) writeAcTrace(ts);
	return(stats_byte[tsi].FxHref); //tsi - time scale (array) index = ts-1!
}
	
double acSKDEEstimator::getFxHhat(int ts){
	double *data=NULL;
	int tsi=ts-1;
	
	stats_byte[tsi].css=meter->getSample(ts, &data); //get the latest sample
	
	stats_byte[tsi].sumKernel=0.0;
	stats_byte[tsi].hHat=bandwidth(ts, data, (int)stats_byte[tsi].css, ep.j);
	
	for(int idx=0; idx<stats_byte[tsi].css; idx++) //TODO: optimize (Herrmann skript)
		stats_byte[tsi].sumKernel+=stdNormalKernel(((ep.x[tsi]-data[idx])/stats_byte[tsi].hHat));

	delete []data;
	stats_byte[tsi].FxHhat=stats_byte[tsi].sumKernel/stats_byte[tsi].css;
	if(ep.trace) writeAcTrace(ts);
	return(stats_byte[tsi].FxHhat);
}

double acSKDEEstimator::stdNormalKernel(double x){
	return(gsl_cdf_ugaussian_P(x));
}

double acSKDEEstimator::bandwidth(int ts, double *data, int css, int j){
	int nn; //squared sample size
	double a=0.0, cst=0.0, sum=0.0, sigHat;
	double r; //roughness estimate
	
	//qsort(data, css, sizeof(double), compar);
	nn=css*css;
	double *delta=new double[nn];

	sigHat=gsl_stats_sd(data, 1, css);
	
	//calculate the differences Yi-Yj
	for(int i=0; i<css; i++)
		for(int j=0; j<css; j++)
			delta[i*css+j]=data[i]-data[j];
		
	//rRef, reference estimate
	//r=gsl_sf_gamma(j+1.5)/(pow(stats_byte[tsi].sigHat, 2.0*j+3.0)*2.0*M_PI);
	r=gsl_sf_gamma(j+1.5)/(pow(sigHat, 2.0*j+3.0)*2.0*M_PI); 
	
	//calcluate the derivatives from higher to lower ones
	double expo=0.0, nom=0.0, den=0.0;
	for(int m=j; m>0; m--){
		nom=pow(2.0, (double)m+0.5)*gsl_sf_gamma(m+0.5);
		den=(M_PI*r*(double)css);
		expo=1.0/(2.0*m+3);
		a=pow(nom/den, expo);
		cst=pow(-1.0, (double)m)/(double)nn;
		for(int idx=0; idx<nn; idx++)
			sum+=hermite((delta[idx]/a), 2*m)*gsl_ran_ugaussian_pdf(delta[idx]/a);
		r=cst*pow(a, -(1.0+2.0*(double)m))*sum;
		sum=0.0;
	}
	
	delete []delta;
	//return bandwidth estimate based on roughness estimation
	return((1/(pow((sqrt(M_PI)*r), 0.3333)))*pow(css, -0.3333));
}

double acSKDEEstimator::hermite(double x, int m){
	double p=x, pp=1.0, u=0.0;
	
	for(int c=2; c<=m; c++){
		u=p; 
		p=x*p-(c-1)*pp;
		pp=u;
	}
	return(p);
}	

acPolicy *acSKDEEstimator::getPolicy(){
	return(policy);
}

acMeter *acSKDEEstimator::getMeter(){
	return(meter);
}

void acSKDEEstimator::writeAcTrace(int ts){
	int tsi=ts-1;
	snprintf(fileName, 128, "acTrace-%i.tr", (getPolicy())->getDscp());
	if((tf=fopen(fileName, "a"))==NULL){
		perror("acEstimator::acEstimator:");
		return; 
	}else{
		if(useHref){
			fprintf(tf, "EST %lf %lf %i %lf %lf %lf %lf\n", \
					Scheduler::instance().clock(), \
					ts*meter->getTau(), ts, \
							stats_byte[tsi].hRef, stats_byte[tsi].FxHref, 1-stats_byte[tsi].FxHref, \
					ep.x[tsi]);
		}else{
			fprintf(tf, "EST %lf %lf %i %lf %lf %lf %lf\n", \
					Scheduler::instance().clock(), \
					ts*meter->getTau(), ts, \
							stats_byte[tsi].hHat, stats_byte[tsi].FxHhat, 1-stats_byte[tsi].FxHhat, \
					ep.x[tsi]);
		}
		fclose(tf);
	}
}

