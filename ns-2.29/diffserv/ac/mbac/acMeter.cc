//
// C++ Implementation: acSKDEMeter
//
// Description: bothom:mbac
//
//
// Author: Thomas Bohnert <tbohnert@dei.uc.pt>, bothom:mbac (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "acEstimator.h"
#include "acPolicy.h"
#include "acMeter.h"
#include "math.h"
#include "string.h"

void MeterTimer::expire(Event *) {
	meter->sampleUpdate(ts);
	resched(meter->getTau()*ts); //reschedule timer
}

acMeter::~acMeter(){}

/* --------  SIMPLE KERNEL DENSITY ESTIMATION (SKDE) algorithm -------- */
acSKDEMeter::acSKDEMeter(acParameterSKDE::skdeMeterParam mp, acEstimator *est=NULL):tcl(Tcl::instance()) {
	this->mp=mp;
	for(int tsi=0; tsi<mp.tsm; tsi++) { //tsi=time scale array index
		bytes[tsi]=0.0;
		head[tsi]=0;
		byteBuf[tsi]=new double[mp.n[tsi]];
		for(int c=0; c<mp.n[tsi]; c++) { //important for online estimation as indication for not-full-sized samples
			byteBuf[tsi][c]=-1.0;
		}
		css[tsi]=0; //set current sample size for this time scale
		mt[tsi]=new MeterTimer(this, tsi+1); //create timer for this time scale
		mt[tsi]->sched(Scheduler::instance().clock()+mp.tau*(tsi+1)); //schedule first event (tsi+1 as array indexes start with zero)
	}
	callBack=false;
	if(est!=NULL) {
		callBack=true; this->est=est;
	}
}

acSKDEMeter::~acSKDEMeter(){}

void acSKDEMeter::meterUpdate(double pktSize) {
	for(int tsi=0; tsi<mp.tsm; tsi++)
		bytes[tsi]+=pktSize;
}

void acSKDEMeter::sampleUpdate(int ts) {
	double prev;
	int tsi=ts-1; //time scale array index

	//current sample size
	if(css[tsi]<mp.n[tsi]) css[tsi]++;

	//update the sample
	prev=byteBuf[tsi][head[tsi]]; //save the oldest value, the arrays are initialized with zeros.
	byteBuf[tsi][head[tsi]]=bytes[tsi]; //write the latest
	head[tsi]=((++head[tsi])%mp.n[tsi]); //simulate a ring buffer

	//for recursive estimators, invoke callback function
	//if(callBack) est->updateStatistics(ts, css[tsi], bytes[tsi], prev, /*inRefCalcFlag*/);
	if(callBack) est->updateStatistics(ts, css[tsi], bytes[tsi], prev);
	if(mp.trace==true) writeAcTrace(DEFAULT, ts, bytes[tsi]);
	bytes[tsi]=0.0; //reset byte counter
	
}

int acSKDEMeter::getSample(int ts, double **bb) {
	int bbi=0, idx=0;
	int tsi=ts-1;

	//check if a full-sized sample has been captured
	if(css[tsi]<mp.n[tsi]) {
		idx=0; //no, so back to the first field in the array
	} else {
		idx=head[tsi]; //yes, so lets do one full lap
	}
	(*bb)=new double[css[tsi]];
	for(bbi=0; bbi<css[tsi]; idx=((idx)%mp.n[tsi])) {
		(*bb)[bbi++]=byteBuf[tsi][idx++];
	}

	//return the number of samples
	return(css[tsi]);
}

double acSKDEMeter::getTau() {
	return(mp.tau);
}

int acSKDEMeter::getTsm() {
	return(mp.tsm);
}

int acSKDEMeter::getSampleSize(int ts) {
	return(mp.n[ts-1]);
}

void acSKDEMeter::printConfig() {
	printf("\tMeter: \
	       \n\t\tWindow length (mp.wlen) = %lf \
	       \n\t\tScaling factor (mp.tsm) = %i \
	       \n\t\tSample period (mp.tau) = %lf \
	       \n\t\tTracing: %s\n",\
	       mp.wlen, mp.tsm, mp.tau, (mp.trace)?("Enabled"):("Disabled"));
}

void acSKDEMeter::writeAcTrace(traceOptions tro, int ts, double bytes) {
	double *bb;
	int size, idx;
	
	switch(tro) {
		case DEFAULT:
			snprintf(fileName, 128, "acTrace-%i.tr", (est->getPolicy())->getDscp());
			if((tf=fopen(fileName, "a"))==NULL) {
				perror("acSKDEMeter::acSKDEMeter:");
				return;
			}
			fprintf(tf, "3 %lf %lf %i %lf\n", Scheduler::instance().clock(), ts*getTau(), ts, bytes);
			fclose(tf);
			break;
		case DUMPSAMPLE:
			snprintf(fileName, 128, "acDump-%i.dmp", (est->getPolicy())->getDscp());
			if((tf=fopen(fileName, "a"))==NULL) {
				perror("acSKDEMeter::acSKDEMeter:");
				return;
			}
			fprintf(tf,"# %.2lf\tSKDE\n", Scheduler::instance().clock());
			for(int tsi=0; tsi<mp.tsm; tsi++){
				size=getSample(tsi+1, &bb);
				fprintf(tf, "## Time scale %i\n", tsi+1);
				for(idx=0; idx<size; idx++)
					fprintf(tf, "%.2lf, ", bb[idx]);
				fprintf(tf, "\n\n");
				delete []bb;
			}	
			fclose(tf);
			break;
		default:
			printf("ERROR: Trace file option unkown\n");
	}
}

void acSKDEMeter::dumpSample() {
	writeAcTrace(DUMPSAMPLE, 0, 0.0);
}

/* --------  FLOYDHB (FHB) algorithm -------- */
acFHBMeter::acFHBMeter(acParameterFHB::fhbMeterParam mp, acEstimator *est=NULL):tcl(Tcl::instance()) {
	this->mp=mp;
	for(int tsi=0; tsi<mp.tsm; tsi++) { //tsi=time scale array index
		bytes[tsi]=0.0;
		head[tsi]=0;
		byteBuf[tsi]=new double[mp.n[tsi]];
		for(int c=0; c<mp.n[tsi]; c++) { //important for online estimation as indication for not-full-sized samples
			byteBuf[tsi][c]=-1.0;
		}
		css[tsi]=0; //set current sample size for this time scale
		mt[tsi]=new MeterTimer(this, tsi+1); //create timer for this time scale
		mt[tsi]->sched(Scheduler::instance().clock()+mp.tau*(tsi+1)); //schedule first event (tsi+1 as array indexes start with zero)
	}
	callBack=false;
	if(est!=NULL) {
		callBack=true; this->est=est;
	}
}

acFHBMeter::~acFHBMeter(){}

void acFHBMeter::meterUpdate(double pktSize) {
	for(int tsi=0; tsi<mp.tsm; tsi++)
		bytes[tsi]+=pktSize;
}

void acFHBMeter::sampleUpdate(int ts) {
	double prev;
	int tsi=ts-1; //time scale array index

	//current sample size
	if(css[tsi]<mp.n[tsi]) css[tsi]++;

	//update the sample
	prev=byteBuf[tsi][head[tsi]]; //save the oldest value, the arrays are initialized with zeros.
	byteBuf[tsi][head[tsi]]=bytes[tsi]; //write the latest
	head[tsi]=((++head[tsi])%mp.n[tsi]); //simulate a ring buffer

	//for recursive estimators, invoke callback function
	//if(callBack) est->updateStatistics(ts, css[tsi], bytes[tsi], prev, /*inRefCalcFlag*/);
	if(callBack) est->updateStatistics(ts, css[tsi], bytes[tsi], prev);
	if(mp.trace==true) writeAcTrace(DEFAULT, ts, bytes[tsi]);
	bytes[tsi]=0.0; //reset byte counter
}

int acFHBMeter::getSample(int ts, double **bb) {
	int bbi=0, idx=0;
	int tsi=ts-1;

	//check if a full-sized sample has been captured
	if(css[tsi]<mp.n[tsi]) {
		idx=0; //no, so back to the first field in the array
	} else {
		idx=head[tsi]; //yes, so lets do one full lap
	}
	(*bb)=new double[css[tsi]];
	for(bbi=0; bbi<css[tsi]; idx=((idx)%mp.n[tsi])) {
		(*bb)[bbi++]=byteBuf[tsi][idx++];
	}

	//return the number of samples
	return(css[tsi]);
}

double acFHBMeter::getTau() {
	return(mp.tau);
}

int acFHBMeter::getTsm() {
	return(mp.tsm);
}

int acFHBMeter::getSampleSize(int ts) {
	return(mp.n[ts-1]);
}

void acFHBMeter::printConfig() {
	printf("\tMeter: \
	       \n\t\tWindow length (mp.wlen) = %lf \
	       \n\t\tScaling factor (mp.tsm) = %i \
	       \n\t\tSample period (mp.tau) = %lf \
	       \n\t\tTracing: %s\n",\
	       mp.wlen, mp.tsm, mp.tau, (mp.trace)?("Enabled"):("Disabled"));
}

void acFHBMeter::writeAcTrace(traceOptions tro, int ts, double bytes) {
	double *bb;
	int size, idx;
	
	switch(tro) {
		case DEFAULT:
			snprintf(fileName, 128, "acTrace-%i.tr", (est->getPolicy())->getDscp());
			if((tf=fopen(fileName, "a"))==NULL) {
				perror("acFHBMeter::acFHBMeter:");
				return;
			}
			fprintf(tf, "3 %lf %lf %i %lf\n", Scheduler::instance().clock(), ts*getTau(), ts, bytes);
			fclose(tf);
			break;
		case DUMPSAMPLE:
			snprintf(fileName, 128, "acDump-%i.dmp", (est->getPolicy())->getDscp());
			if((tf=fopen(fileName, "a"))==NULL) {
				perror("acFHBMeter::acFHBMeter:");
				return;
			}
			fprintf(tf,"# %.2lf\tFHB\n", Scheduler::instance().clock());
			for(int tsi=0; tsi<mp.tsm; tsi++){
				size=getSample(tsi+1, &bb);
				fprintf(tf, "## Time scale %i\n", tsi+1);
				for(idx=0; idx<size; idx++)
					fprintf(tf, "%.2lf, ", bb[idx]);
				fprintf(tf, "\n\n");
				delete []bb;
			}	
			fclose(tf);
			break;
		default:
			printf("ERROR: Trace file option unkown\n");
	}
}

void acFHBMeter::dumpSample() {
	writeAcTrace(DUMPSAMPLE, 0, 0.0);
}

//EOF
