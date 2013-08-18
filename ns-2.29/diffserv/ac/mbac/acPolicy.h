//
// C++ Interface: acPolicy
//
// Description: bothom:mbac
//
//
// Author: Thomas Bohnert <tbohnert@dei.uc.pt>, bothom:mbac (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ACPOLICY_H
#define ACPOLICY_H
#include "packet.h"
#include "acDefinitions.h"
#include "acEstimator.h"
//C++ and STL Headers
#include<set>
#include <vector>
//#include<hash_set>
#include <stdio.h>
#include <math.h>
	//jpoa
#include "dsAdc.h"
//jpoa	

class acPolicy;

//Timer for per flow loss monitoring
class LossMonTimer : public TimerHandler{
protected:
 	acPolicy *policy_;
public:
	LossMonTimer(acPolicy *p) : TimerHandler(), policy_(p) {}
	virtual void expire(Event *);
};

//Timer for periodic flow inactivity checks
class ListTimer : public TimerHandler{
protected:
 	acPolicy *policy_;
public:
	ListTimer(acPolicy *p) : TimerHandler(), policy_(p) {}
	virtual void expire(Event *);
};

class acPolicy{
private:
	Tcl &tcl;
	adcParameterSet aps;	//paramter settings for this AC algorithm
	acEstimator *estimator;
	set<conId, keyCmp> flowList;	//flow management
	set<conId, keyCmp>::iterator it;	//Container iterator
	set<conId, keyCmp>::iterator tmpIt;	//Container iterator
	vector<char> lossMon[MAXPERFLOWSTATS]; //vector for per flow loss statistics
	vector<char>::iterator vit;

	ListTimer *listTimer;
	LossMonTimer *lmt;
	int dts; //dominant time scale
	char fileName[128];
	FILE *tf; //trace file for evaluation
	void writeAcTrace(admissionControlType act, bool admitt);
	double calcDipeFactor();
	class dsAdc *a;
public:
	acPolicy();
	acPolicy(adcParameterSet aps);
	~acPolicy();
	bool admissionRequest(Packet *pkt);
	bool newFlowDetection(conId con);
	bool ssumAlgorithm(conId con);
	bool skdeAlgorithm(conId con);
	bool oqaAlgorithm(conId con);
	bool fhbAlgorithm(conId con);
	void flowInActivityCheck();
	double getCheckPeriod();
	void stopSource(Packet *pkt);
	void dropEventHandler(int fid);
	int getDscp();
	double getCapacity();
	double getPeakRate();
	double getBufferSize();
	double getPktSize();
	int getNumActiveFlows();
	int getMaxNumFlows();
	admissionControlType getAdcType();
	void printAdcSetup();
	void dumpLossMonitor();
};

#endif

