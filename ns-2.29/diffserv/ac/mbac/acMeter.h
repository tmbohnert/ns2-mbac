//
// C++ Interface: acSKDEMeter
//
// Description: bothom:mbac
//
//
// Author: Thomas Bohnert <tbohnert@dei.uc.pt>, bothom:mbac (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ACMETER_H
#define ACMETER_H
#include "acEstimator.h"
#include "acDefinitions.h"
#include "timer-handler.h"

enum traceOptions{DEFAULT, DUMPSAMPLE};

class acSKDEMeter;
class acEstimator;
class acMeter;

//Timer for metering/measurement
class MeterTimer : public TimerHandler{
protected:
	acMeter *meter;
	int ts; //time scale multiplier
public:
	MeterTimer(acMeter *m, int ts) : TimerHandler(), meter(m), ts(ts) {}
	void expire(Event *);
};

class acMeter{
public:
	virtual ~acMeter();
	virtual void writeAcTrace(traceOptions tro, int ts, double bytes)=0;
	virtual void meterUpdate(double pktSize)=0;
	virtual void sampleUpdate(int ts)=0;
	virtual int getSample(int ts, double **bb)=0;
	virtual double getTau()=0;
	virtual int getTsm()=0;
	virtual int getSampleSize(int ts)=0;
	virtual void dumpSample()=0;
	virtual void printConfig()=0;
};

/* --------  SIMPLE KERNEL DENSITY ESTIMATION (SKDE) algorithm -------- */
class acSKDEMeter : public acMeter{
private:
	Tcl &tcl;
	acParameterSKDE::skdeMeterParam mp;
	acEstimator *est;
	bool callBack;
	double bytes[MAXTIMESCALES]; //byte counter
	MeterTimer *mt[MAXTIMESCALES];
	double tau; //measurement intervall in [s]
	double wlen;
	double *byteBuf[MAXTIMESCALES];
	int css[MAXTIMESCALES]; //keep track of current sample size (initial phase of the MBAC algorithm)
	int head[MAXTIMESCALES]; //buffer index
	//mbac:RG
	//change size to 128 because was only 15 but 128 in the function dumpSample used
	char fileName[128];
	//mbac:RG
	FILE *tf; //trace file for debugging/evaluation
	traceOptions trOpt;
	void writeAcTrace(traceOptions tro, int ts, double bytes);
public:
	acSKDEMeter(acParameterSKDE::skdeMeterParam mp, acEstimator *est);
	~acSKDEMeter();
	void meterUpdate(double pktSize);
	void sampleUpdate(int ts);
	int getSample(int ts, double **bb);
	double getTau();
	int getTsm();
	int getSampleSize(int ts);
	void dumpSample();
	void printConfig();
};

/* --------  FLOYDHB (FHB) algorithm -------- */
class acFHBMeter : public acMeter{
private:
	Tcl &tcl;
	acParameterFHB::fhbMeterParam mp;
	acEstimator *est;
	bool callBack;
	double bytes[MAXTIMESCALES]; //byte counter
	MeterTimer *mt[MAXTIMESCALES];
	double tau; //measurement intervall in [s]
	double wlen;
	double *byteBuf[MAXTIMESCALES];
	int css[MAXTIMESCALES]; //keep track of current sample size (initial phase of the MBAC algorithm)
	int head[MAXTIMESCALES]; //buffer index
	//mbac:RG
	//change size to 128 because was only 15 but 128 in the function dumpSample used
	char fileName[128];
	//mbac:RG
	FILE *tf; //trace file for debugging/evaluation
	traceOptions trOpt;
	void writeAcTrace(traceOptions tro, int ts, double bytes);
public:
	acFHBMeter(acParameterFHB::fhbMeterParam mp, acEstimator *est);
	~acFHBMeter();
	void meterUpdate(double pktSize);
	void sampleUpdate(int ts);
	int getSample(int ts, double **bb);
	double getTau();
	int getTsm();
	int getSampleSize(int ts);
	void dumpSample();
	void printConfig();
};

#endif
