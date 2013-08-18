//
// C++ Implementation: acPolicy
//
// Description: bothom:mbac
//
//
// Author: Thomas Bohnert <tbohnert@dei.uc.pt>, bothom:mbac (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "acPolicy.h"
#include "acEstimator.h"
#include <stdio.h>
#include <string.h>

void ListTimer::expire(Event *){
	policy_->flowInActivityCheck();
	resched(policy_->getCheckPeriod());
}

acPolicy::acPolicy(adcParameterSet aps):tcl(Tcl::instance()){
	this->aps=aps; //save parameters
	listTimer=new ListTimer(this); //create timer
	listTimer->sched(aps.pp.checkPeriod); //schedule first event
	switch(aps.ep.estType){
		case NLEST : estimator=NULL; break;// SSUM
		case SKDEEST : estimator=new acSKDEEstimator(this, aps.ep.acpskde->ep, aps.ep.acpskde->mp); break; //SKDE
		default : printf("ERROR: No estimator found, estTpye unknown\n");
	};
	//initial file open for tracing support
	if(aps.pp.trace || aps.ep.acpskde->ep.trace || aps.ep.acpskde->mp.trace){
		snprintf(fileName, 128, "acTrace-%i.tr", getDscp());
		if((tf=fopen(fileName, "w"))==NULL){
			perror("acPolicy::acPolicy:");
		}
		fclose(tf);
	}
}

acPolicy::~acPolicy(){
	if(tf){fclose(tf);}
}

bool acPolicy::admissionRequest(Packet *pkt){
	if(DEBUGG) printf("DEBUGG: Calling acPolicy::admissionRequest()\n");
	hdr_ip* iph; hdr_cmn* cmh; conId con;
	bool admitt=false;

	//flow identification
	cmh=hdr_cmn::access(pkt);
	iph=hdr_ip::access(pkt);
	con.src=iph->src();
	con.dst=iph->dst();
	con.prio=iph->prio();
	con.fid=iph->flowid();
	con.timeStamp=Scheduler::instance().clock(); //timestamp used to monitor flow/connection acitvity
	con.genConHash();
	
	//new or known flow?
	if(newFlowDetection(con)==false) {
		cmh->setAcMark(0); //mark packet for dropping statistics, zero means packet not dropped due to AC
		//update the meter (packet size in byte)
		if(estimator)	estimator->meterUpdate(cmh->size()); 
		return(true); //flow with a valid admission token
	}
	
	//ok, it is a new flow. Lets see if we can admitt it.
	switch(aps.adcType){
		case SSUM	: if(DEBUG) printf("\n\n%.2lf: New flow detected, calling SSUM admission algorithm\n", Scheduler::instance().clock());
								admitt=ssumAlgorithm(con);
								break;
		case SKDE	:	if(DEBUG) printf("\n\n%.2lf: New flow detected, calling SKDE admission algorithm\n", Scheduler::instance().clock());
								admitt=skdeAlgorithm(con);
								break;
		default :		printf("ERROR: No AC algotithm found, adcTpye unknown\n");
								admitt=false;
	}
	
	if(!admitt){
		cmh->setAcMark(-1); //mark packet for dropping statistics, packet dropped due to AC
		stopSource(pkt); //shut source down (send an ICMP "Network unavailable" message)
	}else{
		cmh->setAcMark(0);
	}
		
	return(admitt);
}

bool acPolicy::newFlowDetection(conId con)	{
	//check if this flow has been previously admitted
	it=flowList.find(con);
	if(it!=flowList.end()) {
		//update addmission timestamp. As long a flow is active we do not touch it!
		flowList.erase(it);
		flowList.insert(con); //includes the latest timestamp
		return(false); //no new flow => false
	} else {
		return(true);
	}
}

bool acPolicy::ssumAlgorithm(conId con){
	bool admitt=false; //we are pessimists

	if(aps.pp.numFlows<aps.pp.maxFlows) {
		flowList.insert(con); //insert flow in list of admitted flows
		aps.pp.numFlows=flowList.size(); //update the number of admitted flows
		admitt=true;
	}
	
	if(DEBUG) printf("Class %i::SSUM (numFlows = %i, maxFlowsSS = %i) -> Flow at %.2lf %s\n", aps.dscp, aps.pp.numFlows,aps.pp.maxFlows, Scheduler::instance().clock(), (admitt)?("Admitted"):("Rejected"));
	if(aps.pp.trace) writeAcTrace(SSUM, admitt);

	if(admitt)
		return(true);
	else
		return(false);
}

bool acPolicy::skdeAlgorithm(conId con){
	bool admitt=false; //we are pessimists
	double pLossHat=0.0, pLossHatMax=0.0;

	if(aps.pp.numFlows<aps.pp.maxFlows){
		//no congestion as the link is not fully loaded
		if(DEBUG) printf("Class %i::SKDE - Charging stage: MaxNumFlows = %i\n", aps.dscp, aps.pp.maxFlows);
		flowList.insert(con); //insert flow in list of admitted flows
		aps.pp.numFlows=flowList.size(); //update the number of admitted flows
		admitt=true;
	}else{
		if(DEBUG) printf("Class %i::SKDE - Saturation stage\n", aps.dscp);
		dts=0;
		for(int ts=1; ts<=aps.ep.acpskde->mp.tsm; ts++){
			pLossHat=1-estimator->estimate(ts);
			if(pLossHatMax<pLossHat){
				pLossHatMax=pLossHat;
				dts=ts;
			}	
			if(pLossHatMax>aps.pp.pLoss){ //TODO: What is the difference between greater and greaterEqual
				aps.pp.numRejects++;
				admitt=false;
				break;				
			}
			admitt=true;
		}
		//if(DEBUGGG) estimator->getMeter()->dumpSample();
	}
	
	if(admitt){
		flowList.insert(con); //insert flow in list of admitted flows
		aps.pp.numFlows=flowList.size(); //update the number of admitted flows
	}

	if(DEBUG) printf("Class %i::SKDE (pLoss = %.5lf, pLoosHatMax = %.5lf) -> Flow at %.2lf %s\n\tDTS = %i\n\tNumber admissions = %i\n\tNumber rejections = %i", aps.dscp, aps.pp.pLoss, pLossHatMax, Scheduler::instance().clock(), (admitt)?("Admitted"):("Rejected"), dts, aps.pp.numFlows, aps.pp.numRejects);
	if(aps.pp.trace) writeAcTrace(SKDE, admitt);
	return(admitt);
}

void acPolicy::flowInActivityCheck(){
	int c=0;
	double now=Scheduler::instance().clock();
	for(it=flowList.begin(); it!=flowList.end(); it++) {
		if((now-it->timeStamp)>=aps.pp.flowTimeout){
			flowList.erase(it);
			c++;
		}
	}
	if(DEBUG) printf("\n\n%.2lf: Policy Flow Activity Monitor\nExpired admissions %i\nNext revision at %.2lf\n", now, c, now+aps.pp.checkPeriod);
	aps.pp.numFlows=flowList.size(); //update the number of admitted flows
}

void acPolicy::stopSource(Packet *pkt){
	Agent *agent;
	hdr_cmn* cmh=hdr_cmn::access(pkt);
	agent=(Agent *)cmh->srcPointer();
	agent->close();
}

double acPolicy::getCheckPeriod(){
	return(aps.pp.checkPeriod);
}

int acPolicy::getDscp(){
	return(aps.dscp);
}

void acPolicy::printAdcSetup(){
	printf("Selected MBAC algorithm: adcType = ");
	switch(aps.adcType){
		case SSUM: printf("SSUM\n"); break;
		case SKDE: printf("SKDE\n"); break;
		default: printf("ERROR: Unknown adcType\n");
	}
	printf("\tPolicy Module:\n\t\tpolType = ");
	switch(aps.pp.polType){
		case ACCEPT: 	printf("ACCEPT\n"); break;
		case REPLACE: printf("REPLACE\n"); break;
		default: printf("ERROR: Unknown polType\n");
	}
	printf("\t\tNetwork & Source settings:\n\t\t\tcapacity = %.1f bps\n\t\t\tbufferSize = %.1f bit\n\t\t\tpeakRate = %.1f bps\n\t\t\tpLoss = %f\n", aps.pp.capacity, aps.pp.bufferSize, aps.pp.peakRate, aps.pp.pLoss);
	printf("\t\tFlow managment:\n\t\t\tcheckPeriod = %.2f\n\t\t\tflowTimerout = %.2f\n", aps.pp.checkPeriod, aps.pp.flowTimeout);
	printf("\t\tTracing: %s\n", (aps.pp.trace)?("Enabled"):("Disabled"));
	
	if(estimator!=NULL) estimator->printConfig();
}

void acPolicy::writeAcTrace(admissionControlType act, bool admitt){
	snprintf(fileName, 128, "acTrace-%i.tr", getDscp());
	if((tf=fopen(fileName, "a"))==NULL){
		perror("acPolicy::acPolicy:");
		return; 
	}else{
		switch(act){
			case SSUM:	fprintf(tf, "POL %lf %s %i\n", Scheduler::instance().clock(), (admitt)?("Admitted"):("Rejected"), aps.pp.numFlows);
									break;
									case SKDE:	fprintf(tf, "POL %lf %s %i %i\n", Scheduler::instance().clock(), (admitt)?("Admitted"):("Rejected"), dts, aps.pp.numFlows);
									break;
				default:	printf("ERROR: Unknown admission control type, no trace written\n");
									break;
		}							
		fclose(tf);
	}
}
