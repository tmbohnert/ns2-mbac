//
// C++ Interface: dsAdc
//
// Description: bothom:mbac
//
//
// Author: Thomas Bohnert <tbohnert@dei.uc.pt>, bothom:mbac (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DSADC_H
#define DSADC_H

// //jpoa
// #include "dsEdge.h"
// //jpoa
#include "packet.h"
#include "rng.h" //MAXINT
#include "dsred.h"
#include "acDefinitions.h"
#include "acPolicy.h"
#include <math.h>

//MAX_QUEUES limits the number of physical queues, i.e. the number of traffic classes
#define MAX_ADCS MAX_QUEUES

class dsAdc : public TclObject {
private:
	Tcl &tcl;
	//different ACs for different classes possible, a single one for each class.
	adcPoolEntry adcPool[MAX_ADCS];
	int adcPoolIndex;
	//query the adc pool
	int lookupAdcPool(int dscp);
	bool adcDebug;
public:
	//jpoa
	class edgeQueue *e; //TODO: public?
	dsAdc();
	~dsAdc();
	void addAdcEntry(int argc, const char*const* argv);
 	void setQueueReference(edgeQueue *edge);
	policyType configPolicy(const char*const* argv, int idx);
	bool admissionRequest(Packet *pkt);
	void pktDropEvent(int dscp, int fid);
	void printAdcSetup();
};

#endif