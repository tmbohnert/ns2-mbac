//
// C++ Interface: acMeter
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

class acMeter;
class acEstimator;

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
	acMeter(acParameterSKDE::skdeMeterParam mp, acEstimator *est);
	~acMeter();
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
