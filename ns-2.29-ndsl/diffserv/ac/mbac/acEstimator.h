//
// C++ Interface: acEstimator
//
// Description: bothom:mbac
//
//
// Author: Thomas Bohnert <tbohnert@dei.uc.pt>, bothom:mbac (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ACESTIMATOR_H
#define ACESTIMATOR_H
#include "acDefinitions.h"
#include "acMeter.h"


struct skdeStats{
	int n; double css; //total and current sample size
	double sum, sqrSum, myHat, sigHat; 
	double sumRefKernel, hRef, FxHref, FxHrefRec[30];
	double sumKernel, hHat, FxHhat, FxHhatRec[30];
	//double sumRefKernel, hRef, FxHref;
	//double sumKernel, hHat, FxHhat;
	int idxHref, idxHhat;
	
	void init(){
		css=0.0;
		sum=sqrSum=myHat=sigHat=0.0;
		sumRefKernel=hRef=0.0; FxHref=1.0;
		sumKernel=hHat=0.0; FxHhat=1.0;
		for(int c=0; c<30; c++)
			FxHrefRec[c]=FxHhatRec[c]=1.0;
		idxHref=idxHhat=0;
	}
};

class acEstimator{
protected:
	acPolicy *policy;
	acMeter *meter;
	char fileName[128];
	FILE *tf; //trace file for debugging/evaluation
	virtual void writeAcTrace(int ts)=0;
		
public:
	virtual ~acEstimator();
	virtual void printConfig()=0;
	virtual void meterUpdate(double bytes)=0;
	virtual void updateStatistics(int ts, int css, double latestVol, double oldestVol)=0;
	virtual double estimate(int ts)=0;
	virtual double intEstimate(int ts)=0;
	virtual acPolicy *getPolicy()=0;
	virtual acMeter *getMeter()=0;
};

class acSKDEEstimator : public acEstimator {
private:
	Tcl &tcl;
	acParameterSKDE::skdeEstParam ep; //estimator parameter
	skdeStats *stats_byte;
	bool useHref; //use estimated optimal bandwidth
	void updateGaussRefEst(int ts, double latest, double oldest);
	double getFxHref(int ts);
	double getFxHhat(int ts);
	double stdNormalKernel(double x);
	double bandwidth(int ts, double *data, int css, int j);
	double hermite(double x, int m);
	void writeAcTrace(int ts);
public:
	acSKDEEstimator(acPolicy *policy, acParameterSKDE::skdeEstParam ep, acParameterSKDE::skdeMeterParam mp);
	void printConfig();
	void meterUpdate(double bytes);
	void updateStatistics(int ts, int css, double latestPktSize, double oldestPktSize);
	double estimate(int ts);
	double intEstimate(int ts);
	acPolicy *getPolicy();
	acMeter *getMeter();
};

int compDoubleFree(const void *p, const void *s);

#endif
