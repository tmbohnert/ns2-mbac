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

/* --------  SIMPLE KERNEL DENSITY ESTIMATION (SKDE) algorithm -------- */
acSKDEEstimator::acSKDEEstimator(acPolicy *policy, acParameterSKDE::skdeEstParam ep, acParameterSKDE::skdeMeterParam mp):tcl(Tcl::instance()){
	this->policy=policy;
	this->ep=ep;
	meter=new acSKDEMeter(mp, (acEstimator *)this);
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
	//a new measurement has been added to the sample
	//trigger change-point detection
	//changePointDetection();
}

void acSKDEEstimator::updateStatistics(int ts, int css, double latestVol, double oldestVol){
}

void acSKDEEstimator::updateGaussRefEst(int ts, double latest, double oldest){
}

double acSKDEEstimator::estimate(int ts, double cumDf){
	if(ep.intEst){
		return(intEstimate(ts, cumDf));
	}else{
		if(useHref)
			return(getFxHref(ts, cumDf));
		else
			return(getFxHhat(ts, cumDf));
	}
}

double acSKDEEstimator::intEstimate(int ts, double cumDf){
	int tsi=ts-1;
	double qv[30];
	
	if(useHref){
		stats_byte[tsi].FxHrefRec[stats_byte[tsi].idxHref++]=getFxHref(ts, cumDf); //get the latest estimate
		if(stats_byte[tsi].idxHref==30) 
			stats_byte[tsi].idxHref=0;
		memcpy(qv, stats_byte[tsi].FxHrefRec, 30*sizeof(double)); //copy it to a working vector to protect the record
	}else{
		stats_byte[tsi].FxHhatRec[stats_byte[tsi].idxHhat++]=getFxHhat(ts, cumDf);
		if(stats_byte[tsi].idxHhat==30)
			stats_byte[tsi].idxHhat=0;
		memcpy(qv, stats_byte[tsi].FxHhatRec, 30*sizeof(double));
	}
	qsort(qv, 30, sizeof(double), compDoubleFree);
	return(qv[3]);
}

double acSKDEEstimator::getFxHref(int ts, double cumDf){
	double *data=NULL;
	int tsi=ts-1;
	double x=0.0;
	
		// If DIPE is enabled, get the new X value
	if(cumDf)
		 x=calcDipeX(ts, cumDf);
	else
		x=ep.x[tsi];
	
	stats_byte[tsi].css=meter->getSample(ts, &data); //get the latest sample
	stats_byte[tsi].sigHat=gsl_stats_sd(data, 1, (int)stats_byte[tsi].css);

	stats_byte[tsi].sumRefKernel=0.0;
	stats_byte[tsi].hRef=stats_byte[tsi].sigHat*pow(4.0, 0.333)*pow(stats_byte[tsi].css, -0.3333);
	for(int idx=0; idx<stats_byte[tsi].css; idx++) //TODO: optimize (Herrmann skript)
		stats_byte[tsi].sumRefKernel+=stdNormalKernel(((x-data[idx])/stats_byte[tsi].hRef));
		//stats_byte[tsi].sumRefKernel+=stdNormalKernel(((ep.x[tsi]-data[idx])/stats_byte[tsi].hRef));

	delete []data;
	stats_byte[tsi].FxHref=stats_byte[tsi].sumRefKernel/(stats_byte[tsi].css);
	if(ep.trace) writeAcTrace(ts);
	return(stats_byte[tsi].FxHref); //tsi - time scale (array) index = ts-1!
}
	
double acSKDEEstimator::getFxHhat(int ts, double cumDf){
	double *data=NULL;
	int tsi=ts-1;
	double x=0.0;
	
	// If DIPE is enabled, get the new X value
	if(cumDf)
		x=calcDipeX(ts, cumDf);
	else
		x=ep.x[tsi];
	
	stats_byte[tsi].css=meter->getSample(ts, &data); //get the latest sample
	
	stats_byte[tsi].sumKernel=0.0;
	stats_byte[tsi].hHat=bandwidth(ts, data, (int)stats_byte[tsi].css, ep.j);
	
	for(int idx=0; idx<stats_byte[tsi].css; idx++) //TODO: optimize (Herrmann skript)
		stats_byte[tsi].sumKernel+=stdNormalKernel(((x-data[idx])/stats_byte[tsi].hHat));
		//stats_byte[tsi].sumKernel+=stdNormalKernel(((ep.x[tsi]-data[idx])/stats_byte[tsi].hHat));

	delete []data;
	stats_byte[tsi].FxHhat=stats_byte[tsi].sumKernel/stats_byte[tsi].css;
	if(ep.trace) writeAcTrace(ts);
	return(stats_byte[tsi].FxHhat);
}

double acSKDEEstimator::calcDipeX(int ts, double cumDf){
	double c=getPolicy()->getCapacity();
	double p=getPolicy()->getPeakRate();
	double omega=getPolicy()->getBufferSize();
	
	return((c-(1+cumDf)*p)*ts+omega);
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
		perror("acSKDEEstimator::acSKDEEstimator:");
		return; 
	}else{
		if(useHref){
			fprintf(tf, "2 %lf %lf %i %lf %lf %lf %lf\n", \
					Scheduler::instance().clock(), \
					ts*meter->getTau(), ts, \
							stats_byte[tsi].hRef, stats_byte[tsi].FxHref, 1-stats_byte[tsi].FxHref, \
					ep.x[tsi]);
		}else{
			fprintf(tf, "2 %lf %lf %i %lf %lf %lf %lf\n", \
					Scheduler::instance().clock(), \
					ts*meter->getTau(), ts, \
							stats_byte[tsi].hHat, stats_byte[tsi].FxHhat, 1-stats_byte[tsi].FxHhat, \
					ep.x[tsi]);
		}
		fclose(tf);
	}
}

void acSKDEEstimator::changePointDetection(int ts){
	return;
}

/* --------  FLOYDHB (FHB) algorithm -------- */
acFHBEstimator::acFHBEstimator(acPolicy *policy, acParameterFHB::fhbEstParam ep, acParameterFHB::fhbMeterParam mp):tcl(Tcl::instance()){
	this->policy=policy;
	this->ep=ep;
	meter=new acFHBMeter(mp, (acEstimator *)this);
	stats_byte=new fhbStats[mp.tsm]; //time scale individual statistics
	for(int tsi=0; tsi<mp.tsm; tsi++) { //tsi=time scale array index
		stats_byte[tsi].init();
	}
}

void acFHBEstimator::printConfig(){
	printf("\tEstimator:\n\t\testType = FHBEST\n\t\tEMWA weights (ep.w[]) = ");
	for(int tsi=0; tsi<meter->getTsm(); tsi++)
		printf("%lf ", ep.w[tsi]);
	printf("\n\t\tChange Point Extension (CPE) enabled = %s", (ep.cpe)?("Enabled"):("Disabled"));
	printf("\n\t\t\tParamter K=%lf\n\t\t\tParamter Lambda=%lf", ep.k, ep.lmbd);
	printf("\n\t\tTracing: %s\n", (ep.trace)?("Enabled"):("Disabled"));
	
	meter->printConfig();
}

void acFHBEstimator::meterUpdate(double bytes){
	meter->meterUpdate(bytes);
}

void acFHBEstimator::updateStatistics(int ts, int css, double latestVol, double oldestVol){
	int tsi=ts-1;
	if(ep.cpe){ //check if CP extension is enabled
		int n=policy->getNumActiveFlows();
		int nMax=policy->getMaxNumFlows();
		if(n>nMax && cpeInit(ts)==false){ //is cpe initiated?
			stats_byte[tsi].myHat=initChangePointExtension(ts);
			return;
		}
		if(cpeInit(ts)){
			if(updateChangePointExtension(ts, latestVol)){
				stats_byte[tsi].myHat=(1-ep.w[tsi])*stats_byte[tsi].myHat+ep.w[tsi]*latestVol;
			}
		}	
	}else{
		stats_byte[tsi].myHat=(1-ep.w[tsi])*stats_byte[tsi].myHat+ep.w[tsi]*latestVol;	
	}
	return;
}

bool acFHBEstimator::updateChangePointExtension(int ts, double latestVol){
	int tsi=ts-1;
	ep.ewma[tsi]=ep.lmbd*latestVol + (1 - ep.lmbd)*ep.ewma[tsi];
  if((ep.ewma[tsi] > ep.ewmaHighHat[tsi]) || (ep.ewma[tsi] < ep.ewmaLowHat[tsi])){
    printf("There is a change for time scale %i!\n", ts);
		return(true);
	}else{
		return(false);
	}		
}	

bool acFHBEstimator::cpeInit(int ts){
	int tsi=ts-1;
	if(ep.initCpe[tsi])
		return(true);
	else
		return(false); //CPE not yet initiated
}

double acFHBEstimator::initChangePointExtension(int ts){
	int tsi=ts-1;
	double *data=NULL;

	stats_byte[tsi].css=meter->getSample(ts, &data); //get the latest sample
	ep.muHat[tsi]=gsl_stats_mean(data, 1, (int)stats_byte[tsi].css);
	ep.sigHat[tsi]=gsl_stats_sd(data, 1, (int)stats_byte[tsi].css);
	ep.acfOneHat[tsi]=gsl_stats_lag1_autocorrelation(data, 1, (int)stats_byte[tsi].css);
	ep.ewmaHighHat[tsi] = ep.muHat[tsi] + ep.k*ep.sigHat[tsi]*sqrt((ep.lmbd/(2-ep.lmbd))*((1+ep.acfOneHat[tsi]*(1-ep.lmbd)) / (1-ep.acfOneHat[tsi]*(1-ep.lmbd)) ));
	ep.ewmaLowHat[tsi] = ep.muHat[tsi] - ep.k*ep.sigHat[tsi]*sqrt((ep.lmbd/(2-ep.lmbd))*((1+ep.acfOneHat[tsi]*(1-ep.lmbd)) / (1-ep.acfOneHat[tsi]*(1-ep.lmbd)) ));
	ep.ewma[tsi]=ep.muHat[tsi];
	
	delete []data;
	ep.initCpe[tsi]=true;
	if(DEBUG) printf("++ Change Point Detection initiated for timescale %i\n\n", ts);
	return(ep.muHat[tsi]);
}

double acFHBEstimator::estimate(int ts, double cumDf){
	if(ep.trace) writeAcTrace(ts);
	return(stats_byte[ts-1].myHat);
}

double acFHBEstimator::intEstimate(int ts, double cumDf){
	return(0.0);
}

acPolicy *acFHBEstimator::getPolicy(){
	return(policy);
}

acMeter *acFHBEstimator::getMeter(){
	return(meter);
}

void acFHBEstimator::writeAcTrace(int ts){
	int tsi=ts-1;
	snprintf(fileName, 128, "acTrace-%i.tr", (getPolicy())->getDscp());
	if((tf=fopen(fileName, "a"))==NULL){
		perror("acFHBEstimator::acFHBEstimator:");
		return; 
	}else{
		fprintf(tf, "2 %.3lf %.3lf %i %.3lf %.3lf\n", \
			Scheduler::instance().clock(), \
			ts*meter->getTau(), ts, \
			stats_byte[tsi].myHat, ep.w[tsi]);
	}
	fclose(tf);
}

/* --------  Objective QoS Assessment (OQA) algorithm -------- */
void EModelTimer::expire(Event *) {
	est->periodicMIeUpdate();
	resched(to); //reschedule timer
}

acOQAEstimator::acOQAEstimator(acPolicy *policy, acParameterOQA::oqaEstParam ep, acParameterOQA::oqaMeterParam mp):tcl(Tcl::instance()){
	this->policy=policy;
	this->ep=ep;
	ep.eModel.emt=new EModelTimer(this, ep.eModel.to); //TODO: timeout configurable
	ep.eModel.emt->sched(Scheduler::instance().clock()+ep.eModel.to);
}

void acOQAEstimator::printConfig(){
	printf("\tEstimator:\n\t\testType = OQAEST\n\t\ttarget rScore (ep.eModel.rScore) = %lf\n\t\twindow length (ep.eModel.wLen = %lf", ep.eModel.rScore, ep.eModel.wLen);
	printf("\n\t\tTracing = %s\n", (ep.trace)?("Enabled"):("Disabled"));
}

void acOQAEstimator::meterUpdate(double rcv){
/* Attention: This function is called once for each packet and twice if one and the same packet will be discarded */
	if(rcv>0){ //packet received/queued or dropped (loss event)?
		ep.eModel.pkts++; //total packets received counter
	}else{	//we are only interested in loss events. Only then we update the EModel
		double now=Scheduler::instance().clock();				
		double pktRcvSt, pktConRcv, lossRatio=0.0;
		double tst; //time past state transition
		double ie=0.0, mieHat=0.0;
		bool sc=false; //state changed?

		ep.eModel.loss++; // packet lost, update loss counter
		pktConRcv=ep.eModel.pkts-ep.eModel.pktRefLe; //consecutively received packets since last loss event
		pktRcvSt=ep.eModel.pkts-ep.eModel.pktRefSt; //packets received since previous state transition

		//packet drop event, update EModel
		if(ep.eModel.state){ //loss burst state
			if(pktConRcv>=(16*policy->getNumActiveFlows()*0.333)){
				// state has changed meanwhile, update model
				tst=ep.eModel.tple;
				if(pktRcvSt>0)
					lossRatio=(ep.eModel.loss-1)/(pktRcvSt-pktConRcv); //loss rate at the state transition
				ie=lossImpairFactor(lossRatio, 0); 
				//mieHat=updateMeanIe(ie, tst-ep.eModel.tRef, ep.eModel.state, false);
				mieHat=updateMeanIe(pktRcvSt-pktConRcv, ep.eModel.loss-1, lossRatio, ie, (pktRcvSt-pktConRcv)*ep.eModel.frlen, ep.eModel.state, false);
				printf("%lf: Loss Burst -> Gap %lf %lf %lf %lf\n", tst, lossRatio, ie, (pktRcvSt-pktConRcv)*ep.eModel.frlen, mieHat);
				//init loss gap state
				ep.eModel.state=0;
				ep.eModel.loss=0; // reset loss counter
				ep.eModel.tRef=tst; //time reference
				ep.eModel.pktRefSt=ep.eModel.pkts-pktConRcv; //absolute number of packets sent by this state transition
				sc=true;
			}	
		}else{
			if(pktConRcv<=(16*policy->getNumActiveFlows()*0.333)){ //TODO: Activity factor configurable or estimated
				// state has changed meanwhile, update model
				/* an isolated packet packet drop occured within a distance of less than 16 packets */
				tst=ep.eModel.tple;
				if(pktRcvSt>0)
					lossRatio=(ep.eModel.loss-1)/(pktRcvSt-pktConRcv); //loss rate at the state transition
				ie=lossImpairFactor(lossRatio, 0);
				mieHat=updateMeanIe(pktRcvSt-pktConRcv, ep.eModel.loss-1, lossRatio, ie, (pktRcvSt-pktConRcv)*ep.eModel.frlen, ep.eModel.state, false);
				printf("%lf: Loss Gap -> Burst, %lf %lf %lf %lf\n", tst, lossRatio, ie, (pktRcvSt-pktConRcv)*ep.eModel.frlen, mieHat);
				//init loss burst state
				ep.eModel.state=1;
				ep.eModel.loss=0; // reset loss counter
				ep.eModel.tRef=tst; //time reference
				ep.eModel.pktRefSt=ep.eModel.pkts-pktConRcv; //absolute number of packets sent by this state transition
				sc=true;
			}
		}
		//update time and packet reference
		ep.eModel.tple=now;
		ep.eModel.pktRefLe=ep.eModel.pkts;
	}
}

double acOQAEstimator::lossImpairFactor(double lossRatio, int model){
	double x=lossRatio*100;
	switch(model){
		case 0: //polynominal fit, 4th order.
		{
			if(lossRatio<=8) //non-linear part
				return(-0.00094355*x+0.19543*x-1.4758*x+5.1598*x-0.8902*x);
			else
				return(1.286*x-1.786);
			break;
		}
		default: printf("Could'nt find model for EModel Ie fiting\n");
	}
	return(-1);
}

double acOQAEstimator::updateMeanIe(double pkts, double pktsLost, double lossRatio, double ie, double t, int state, bool tmpUpdate){
	double ieb=0.0; double ieg=0.0;
	double blen=0.0; double glen=0.0;
	double ieTwo=0.0;
	double tt=0.0; double check=0; int bc=0; int gc=0;

	// add the latest estimate and update the head pointer (ring buffer)
	if(state==1){
		ep.eModel.ieRec[ep.eModel.ieRecHead].pkts=pkts; //total packets for the duration of this state
		ep.eModel.ieRec[ep.eModel.ieRecHead].pktsLost=pktsLost; //total lost packets for this state
		ep.eModel.ieRec[ep.eModel.ieRecHead].lr=lossRatio; 
		ep.eModel.ieRec[ep.eModel.ieRecHead].ieb=ie;
		ep.eModel.ieRec[ep.eModel.ieRecHead].ieg=0.0;
		ep.eModel.ieRec[ep.eModel.ieRecHead].blen=t;
		ep.eModel.ieRec[ep.eModel.ieRecHead].glen=0.0;
		ep.eModel.ieRec[ep.eModel.ieRecHead].state=state;
	}else{
		ep.eModel.ieRec[ep.eModel.ieRecHead].pkts=pkts;
		ep.eModel.ieRec[ep.eModel.ieRecHead].pktsLost=pktsLost;
		ep.eModel.ieRec[ep.eModel.ieRecHead].lr=lossRatio;
		ep.eModel.ieRec[ep.eModel.ieRecHead].ieg=ie;
		ep.eModel.ieRec[ep.eModel.ieRecHead].ieb=0.0;
		ep.eModel.ieRec[ep.eModel.ieRecHead].glen=t;
		ep.eModel.ieRec[ep.eModel.ieRecHead].blen=0.0;
		ep.eModel.ieRec[ep.eModel.ieRecHead].state=state;
	}
	ep.eModel.ieRecHead=(++ep.eModel.ieRecHead)%(ep.eModel.ieRecTotLen);

	//compute the averages, ieg, ieb, blen and glen over the last T (wLen) sec
	for(int idx=ep.eModel.ieRecHead-1; tt<ep.eModel.wLen && check<=1; idx--){
		if(idx<0){
			idx=ep.eModel.ieRecTotLen-1; //negative idx, jump to the end of the vector
			check++; //maximum two loops
		}
		if(ep.eModel.ieRec[idx].state==1){ //burst state
			ieb+=ep.eModel.ieRec[idx].ieb;
			blen+=ep.eModel.ieRec[idx].blen;
			bc++;
			tt+=ep.eModel.ieRec[idx].blen;
		}
		if(ep.eModel.ieRec[idx].state==0){
			ieg+=ep.eModel.ieRec[idx].ieg;
			glen+=ep.eModel.ieRec[idx].glen;
			gc++;
			tt+=ep.eModel.ieRec[idx].glen;
		}
	}
 	if(bc>0){
		ep.eModel.ieBHat=ieb/bc;
 		ep.eModel.bLenHat=blen/bc;
	}
	if(gc>0){	
		ep.eModel.ieGHat=ieg/gc;
 		ep.eModel.gLenHat=glen/gc;
	}
	
	//calculate mIeHat as in [Raake06] equation 19.
	ieTwo=ep.eModel.ieGHat*(1-exp(-(ep.eModel.gLenHat/22)))+ep.eModel.ieBHat*(1-exp(-(ep.eModel.bLenHat/9)))*exp(-(ep.eModel.gLenHat/22));
	ep.eModel.mieHat=1/(ep.eModel.bLenHat+ep.eModel.gLenHat)*(ep.eModel.ieBHat*ep.eModel.bLenHat+ep.eModel.ieGHat*ep.eModel.gLenHat+9*(ep.eModel.ieBHat-ieTwo)*(exp(-(ep.eModel.bLenHat/9))-1)-22*(ep.eModel.ieBHat-(ep.eModel.ieBHat-ieTwo)*exp(-(ep.eModel.bLenHat/9))-ep.eModel.ieGHat)*(exp(-(ep.eModel.gLenHat/22)-1)));
	//printf("STATE: %i, TT: %lf, IEHAT: %lf, MIEHAT: %lf\n", state, tt, ieHat, ep.eModel.mieHat[codePt]);

	/* this update was triggered by an admission request and not by detecting a state transition */
	if(tmpUpdate){
		ep.eModel.ieRecHeadTmp=ep.eModel.ieRecHead;
		if(--ep.eModel.ieRecHead < 0) //overwrite this record at the next state transition
			ep.eModel.ieRecHead=ep.eModel.ieRecTotLen-1; //correct the head pointer accordingly (ring buffer)
	}else{
		ep.eModel.ieRecHeadTmp=-1;
	}	
	
	if(ep.trace) writeAcTrace(2);
	return(ep.eModel.mieHat);
}

void acOQAEstimator::updateModel(){
		double now=Scheduler::instance().clock();				
		double pktRcvSt, pktConRcv, lossRatio=0.0;
		double tst; //time past state transition
		double ie=0.0, mieHat=0.0;
		bool sc=false; //state changed?

		pktConRcv=ep.eModel.pkts-ep.eModel.pktRefLe; //consecutively received packets since last loss event
		pktRcvSt=ep.eModel.pkts-ep.eModel.pktRefSt; //packets received since previous state transition
		
		//update EModel in case there was no packet loss event recently
		if(ep.eModel.state){ //loss burst state
			if(pktConRcv>=(16*policy->getNumActiveFlows()*0.333)){
				tst=ep.eModel.tple;
				if(pktRcvSt>0)
					lossRatio=(ep.eModel.loss)/(pktRcvSt-pktConRcv); //loss rate at the state transition
				ie=lossImpairFactor(lossRatio, 0); 
				updateMeanIe(pktRcvSt-pktConRcv, ep.eModel.loss, lossRatio, ie, (pktRcvSt-pktConRcv)*ep.eModel.frlen, ep.eModel.state, false);
				printf("%lf: Loss Burst -> Gap %lf %lf %lf %lf\n", tst, lossRatio, ie, (pktRcvSt-pktConRcv)*ep.eModel.frlen, mieHat);
				//init loss gap state
				ep.eModel.state=0;
				ep.eModel.loss=0; // reset loss counter
				ep.eModel.tRef=tst; //time reference
				ep.eModel.pktRefSt=ep.eModel.pkts-pktConRcv; //absolute number of packets sent by this state transition
				sc=true;
			}else{
				//no state transition, we are somewhere withing a loss burst
				tst=now;
				if(pktRcvSt>0){
					lossRatio=(ep.eModel.loss)/(pktRcvSt); //loss rate since last state transition
					ie=lossImpairFactor(lossRatio, 0);
					updateMeanIe(pktRcvSt, ep.eModel.loss, lossRatio, ie, pktRcvSt*ep.eModel.frlen, ep.eModel.state, true);
				}
				sc=false;
			}	
		}
		if(!ep.eModel.state){ //loss gap state
			if(sc){ //if a state transition has been detected before, we have account for the gap since this state transition
				pktConRcv=ep.eModel.pkts-ep.eModel.pktRefSt;
				pktRcvSt=pktConRcv; //packets received since previous state transition
			}else{
				pktConRcv=ep.eModel.pkts-ep.eModel.pktRefLe; //consecutively received packets since last loss event
				//printf("Consecutively received packets since last loss event: %lf\n", pktConRcv);
				pktRcvSt=ep.eModel.pkts-ep.eModel.pktRefSt; //packets received since previous state transition
			}
			tst=now;
			if(pktRcvSt>0){
				lossRatio=(ep.eModel.loss)/(pktRcvSt); //loss rate since last state transition
				ie=lossImpairFactor(lossRatio, 0);
				updateMeanIe(pktRcvSt, ep.eModel.loss, lossRatio, ie, pktRcvSt*ep.eModel.frlen, ep.eModel.state, true);
			}
		}	
}

double acOQAEstimator::singleFlowImpact(){
	double sumLr=0.0;
	double myLrHat=0.0;	
 	double iebPlus=0.0; double iegPlus=0.0;
	double tt=0.0; double check=0; int bc=0; int gc=0;
	double ieTwoPlus=0.0;
	double lostBits=0.0;
	double lostPkts=0.0;
	int head=0;

	//check if updateModel() has detected an unfinished state
	if(ep.eModel.ieRecHeadTmp==-1)
		head=ep.eModel.ieRecHead-1;
	else
		head=ep.eModel.ieRecHeadTmp;			

	//compute the averages for the loss profile over the last T (wLen) sec.
	for(int idx=head; tt<ep.eModel.wLen && check<=1; idx--){
		if(idx<0){
			idx=ep.eModel.ieRecTotLen-1; //negative idx, jump to the end of the vector
			check++; //maximum two loops
		}
		if(ep.eModel.ieRec[idx].state==1){ //burst state
			myLrHat+=ep.eModel.ieRec[idx].lr*ep.eModel.ieRec[idx].blen;
			tt+=ep.eModel.ieRec[idx].blen;
		}
		if(ep.eModel.ieRec[idx].state==0){
			myLrHat+=ep.eModel.ieRec[idx].lr*ep.eModel.ieRec[idx].glen;
			tt+=ep.eModel.ieRec[idx].glen;
		}
		sumLr+=ep.eModel.ieRec[idx].lr;
	}
	myLrHat/=tt; //average loss rate for the loss profile

	//calculate the number of packets lost for a flow at average loss rate,
	//for wlen sec and for an flow activity rate of 3/9 (p_on=3/9)
	lostBits=policy->getPeakRate()*tt*myLrHat*3/9;
	lostPkts=lostBits/ep.eModel.pktSize;

	//compute the averages for the loss profile over the last T sec. (wLen) as if one flow more would be admitted
	for(int idx=head; tt<ep.eModel.wLen && check<=1; idx--){
		if(idx<0){
			idx=ep.eModel.ieRecTotLen-1; //negative idx, jump to the end of the vector
			check++; //maximum two loops
		}

		//caluclate the inflated loss rate and Ie for this state
		ep.eModel.ieRec[idx].lrPlus=(ep.eModel.ieRec[idx].pktsLost+(ep.eModel.ieRec[idx].lr/sumLr)*lostPkts)/ep.eModel.ieRec[idx].pkts;
		ep.eModel.ieRec[idx].iePlus=lossImpairFactor(ep.eModel.ieRec[idx].lrPlus, 0);

		if(ep.eModel.ieRec[idx].state==1){ //burst state
			iebPlus+=ep.eModel.ieRec[idx].iePlus;
			tt+=ep.eModel.ieRec[idx].blen;
			bc++;
		}
		if(ep.eModel.ieRec[idx].state==0){
			iegPlus+=ep.eModel.ieRec[idx].iePlus;
			tt+=ep.eModel.ieRec[idx].glen;
			gc++;
		}
	}
	
 	if(bc>0){
		ep.eModel.ieBHatPlus=iebPlus/bc;
 		//ep.eModel.bLenHat=blen/bc; //preserve the burst/gap profile
	}
	if(gc>0){
		ep.eModel.ieGHatPlus=iegPlus/gc;
 		//ep.eModel.gLenHat=glen/gc; //preserve the burst/gap profile
	}
	
	//calculate mIeHat as in [Raake06] equation 19.
	ieTwoPlus=ep.eModel.ieGHatPlus*(1-exp(-(ep.eModel.gLenHat/22)))+ep.eModel.ieBHat*(1-exp(-(ep.eModel.bLenHat/9)))*exp(-(ep.eModel.gLenHat/22));
	ep.eModel.mieHatPlus=1/(ep.eModel.bLenHat+ep.eModel.gLenHat)*(ep.eModel.ieBHatPlus*ep.eModel.bLenHat+ep.eModel.ieGHatPlus*ep.eModel.gLenHat+9*(ep.eModel.ieBHatPlus-ieTwoPlus)*(exp(-(ep.eModel.bLenHat/9))-1)-22*(ep.eModel.ieBHatPlus-(ep.eModel.ieBHatPlus-ieTwoPlus)*exp(-(ep.eModel.bLenHat/9))-ep.eModel.ieGHatPlus)*(exp(-(ep.eModel.gLenHat/22)-1)));
	//printf("STATE: %i, TT: %lf, IEHAT: %lf, MIEHAT: %lf\n", state, tt, ieHat, ep.eModel.mieHat[codePt]);

	if(ep.trace) writeAcTrace(2);
	return(ep.eModel.mieHatPlus);
}

void acOQAEstimator::periodicMIeUpdate(){
}

void acOQAEstimator::updateStatistics(int ts, int css, double latestVol, double oldestVol){
}

double acOQAEstimator::estimate(int ts, double cumDf){
	updateModel();
	if(ep.trace) writeAcTrace(ts);
	return(ep.eModel.mieHat);
}

double acOQAEstimator::estimate(int ts, double *ars){
	updateModel();
	singleFlowImpact();
	if(ep.trace) writeAcTrace(ts);
	*ars=ep.eModel.mieHat;
	return(ep.eModel.mieHatPlus);
}


double acOQAEstimator::intEstimate(int ts, double cumDf){
	return(0.0);
}

void acOQAEstimator::dropEventHandler(){
	//pseudo meter update (there is no meter defined for this algorithm)
	meterUpdate(0.0); //indicates a single packet loss event (received=0)
}

acPolicy *acOQAEstimator::getPolicy(){
	return(policy);
}

acMeter *acOQAEstimator::getMeter(){
	return(meter);
}

void acOQAEstimator::writeAcTrace(int ts){
	snprintf(fileName, 128, "acTrace-%i.tr", (getPolicy())->getDscp());
	if((tf=fopen(fileName, "a"))==NULL){
		perror("acOQAEstimator::acOQAEstimator:");
		return; 
	}else{
		switch(ts){
			case 1: fprintf(tf, "2 %.3lf %.3lf\n", Scheduler::instance().clock(), ep.eModel.mieHat);
							break;
			case 2: fprintf(tf, "2 %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf\n", Scheduler::instance().clock(), ep.eModel.mieHat, ep.eModel.ieBHatPlus, ep.eModel.bLenHat, ep.eModel.ieGHatPlus, ep.eModel.gLenHat);
							break;
		 default: printf("ERROR: unknown case, see \"acOQAEstimator::writeAcTrace()\"\n");
							break;
		}
	}	
	fclose(tf);
}

