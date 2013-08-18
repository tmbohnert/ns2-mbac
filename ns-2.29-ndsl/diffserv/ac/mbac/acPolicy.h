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
#include <stdio.h>

class acPolicy;

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
	set <conId, keyCmp> flowList;	//flow management
	set	<conId, keyCmp>::iterator it;	//Container iterator
	ListTimer *listTimer;
	int dts; //dominant time scale
	char fileName[128];
	FILE *tf; //trace file for evaluation
	void writeAcTrace(admissionControlType act, bool admitt);
public:
	acPolicy();
	acPolicy(adcParameterSet aps);
	~acPolicy();
	bool admissionRequest(Packet *pkt);
	bool newFlowDetection(conId con);
	bool ssumAlgorithm(conId con);
	bool skdeAlgorithm(conId con);
	void flowInActivityCheck();
	double getCheckPeriod();
	void stopSource(Packet *pkt);
	int getDscp();
	void printAdcSetup();	
};

#endif
