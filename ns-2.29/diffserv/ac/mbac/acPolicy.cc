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

void LossMonTimer::expire(Event *){
	policy_->dumpLossMonitor();
}
		
void ListTimer::expire(Event *){
	policy_->flowInActivityCheck();
	resched(policy_->getCheckPeriod());
}

acPolicy::acPolicy(adcParameterSet aps):tcl(Tcl::instance()){
	bool openTraceFile=false;
	this->aps=aps; //save parameters (element wise copy)
	if(aps.pp.flowMon){ //when to dump the per flow records, if enabled
		lmt=new LossMonTimer(this); //create timer
		lmt->sched(aps.pp.dtime); //schedule first event
	}
	listTimer=new ListTimer(this); //create timer
	listTimer->sched(aps.pp.checkPeriod); //schedule first event

	switch(aps.ep.estType){
		case NLEST	 :	estimator=NULL;
										if(aps.pp.trace)
											openTraceFile=true;
										break;// SSUM
											
		case SKDEEST :	estimator=new acSKDEEstimator(this, aps.ep.acpskde->ep, aps.ep.acpskde->mp);
										if(aps.pp.trace || aps.ep.acpskde->ep.trace || aps.ep.acpskde->mp.trace)
											openTraceFile=true;
										break; //SKDE

		case FHBEST :		estimator=new acFHBEstimator(this, aps.ep.acpfhb->ep, aps.ep.acpfhb->mp);
										if(aps.pp.trace || aps.ep.acpfhb->ep.trace || aps.ep.acpfhb->mp.trace)
											openTraceFile=true;
										break; //FHB

		case OQAEST :		estimator=new acOQAEstimator(this, aps.ep.acpoqa->ep, aps.ep.acpoqa->mp);
										if(aps.pp.trace || aps.ep.acpoqa->ep.trace)
											openTraceFile=true;
										break; //OQA

		default : printf("ERROR: No estimator found, estTpye unknown\n");
	};
	
	//initial file open for tracing support
	if(openTraceFile){
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
	con.at=0.0;
	con.timeStamp=Scheduler::instance().clock(); //timestamp used to monitor flow/connection acitvity
	con.genConHash();
	
	//new or known flow?
	if(newFlowDetection(con)==false) {
		cmh->setAcMark(0); //mark packet for dropping statistics, zero means packet has not been not dropped due to AC
		//update the meter (packet size in byte)
		if(estimator)	estimator->meterUpdate(cmh->size());
		//update per flow loss statistics
		
		if(aps.pp.flowMon && aps.pp.flowMonMinID <= con.fid && con.fid <= aps.pp.flowMonMaxID)
			lossMon[con.fid].push_back('+'); //indiactes a successfully received packet //TODO: Range configurable
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
		case FHB	:	if(DEBUG) printf("\n\n%.2lf: New flow detected, calling FHB admission algorithm\n", Scheduler::instance().clock());
								admitt=fhbAlgorithm(con);
								break;
		case OQA	:	if(DEBUG) printf("\n\n%.2lf: New flow detected, calling OQA admission algorithm\n", Scheduler::instance().clock());
								admitt=oqaAlgorithm(con);
								break;
		default :		printf("ERROR: No AC algorithm found, adcTpye unknown\n");
								admitt=false;
	}
	
	if(!admitt){
		cmh->setAcMark(-1); //mark packet for dropping statistics, packet dropped due to AC
		stopSource(pkt); //shut source down (send an ICMP "Network unavailable" message)
	}else{
		cmh->setAcMark(0);
		if(aps.pp.flowMon && aps.pp.flowMonMinID <= con.fid && con.fid <= aps.pp.flowMonMaxID
) lossMon[con.fid].push_back('+'); //indiactes a successfully received packet //TODO: Range configurable
	}
		
	return(admitt);
}

bool acPolicy::newFlowDetection(conId con)	{
	//check if this flow has been previously admitted
	it=flowList.find(con);
	if(it!=flowList.end()) {
		con.at=it->at;  //update addmission timestamp 
		flowList.erase(it);
		flowList.insert(con); //includes the latest activity timestamp
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
	bool admitt=false; //admission flag
	double pLossHat=0.0, pLossHatMax=0.0, cumDf=0.0;
	double now=Scheduler::instance().clock();
	bool iQAdmitt=false;

	if(aps.pp.numFlows<aps.pp.maxFlows){
		//no congestion as the link is not fully loaded
		if(DEBUG) printf("Class %i::SKDE - Charging stage: MaxNumFlows = %i\n", aps.dscp, aps.pp.maxFlows);
		iQAdmitt=true; //admitt the flow
	}else{
		if(DEBUG) printf("Class %i::SKDE - Saturation stage\n", aps.dscp);
		dts=0;
		
		//account for delayed perception of newly admitted flows impact
		if(aps.pp.dipe)	cumDf=calcDipeFactor(); //TODO: DELAYFACTOR ON/OFF INCOMPLETE
		
		//check for resources
		for(int ts=1; ts<=aps.ep.acpskde->mp.tsm; ts++){
			pLossHat=1-estimator->estimate(ts, cumDf);
			if(pLossHatMax<pLossHat){
				pLossHatMax=pLossHat;
				dts=ts;
			}
			if(pLossHatMax>aps.pp.pLoss){ //TODO: What is the difference between greater and greaterEqual
				iQAdmitt=false;
				break;				
			}
			iQAdmitt=true;
		}
	//if(DEBUGGG) estimator->getMeter()->dumpSample();
	}

	if(iQAdmitt)
		admitt=true;
	else
		admitt=false;
	
	if(admitt){	
		con.at=now;
		flowList.insert(con); //insert flow in list of admitted flows
		aps.pp.numFlows=flowList.size(); //update the number of admitted flows
	}else{
		aps.pp.numRejects++;
	}	

	if(DEBUG) printf("Class %i::SKDE (pLoss = %.5lf, pLoosHatMax = %.5lf) -> Flow at %.2lf %s\n\tDTS = \
				%i\n\tNumber admissions = %i\n\tNumber rejections = %i\n\n", aps.dscp, aps.pp.pLoss, pLossHatMax, \
				now, (admitt)?("Admitted"):("Rejected"), dts, aps.pp.numFlows, aps.pp.numRejects);

	if(aps.pp.trace) writeAcTrace(SKDE, admitt);
	return(admitt);
}

bool acPolicy::oqaAlgorithm(conId con){
	bool admitt=false; //we are pessimists
	double now=Scheduler::instance().clock();
	double ars=0.0;;

	if(aps.pp.numFlows<aps.pp.maxFlows){
		//no congestion as the link is not fully loaded
		if(DEBUG) printf("Class %i::OQA - Charging stage: MaxNumFlows = %i\n", aps.dscp, aps.pp.maxFlows);
		admitt=true;
	}else{
		if(DEBUG) printf("Class %i::OQA - Saturation stage\n", aps.dscp);
		//get rScoreHat. Mind that the estimator has it's own copy of the estimator parameters. Hence, we have to ask it by calling est->estimate()
		if(!aps.pp.prede){
			aps.ep.acpoqa->ep.eModel.rScoreHat=(94-(4+1*(aps.pp.bufferSize/aps.pp.capacity))-estimator->estimate(0, 0.0));
			ars=aps.ep.acpoqa->ep.eModel.rScoreHat;
		}else{
			//Here I am, can't overload virtual function, what a fuck!
			aps.ep.acpoqa->ep.eModel.rScoreHat=(94-(4+1*(aps.pp.bufferSize/aps.pp.capacity))-estimator->estimate(0, &ars));
		}
		if(aps.ep.acpoqa->ep.eModel.rScoreHat>=aps.ep.acpoqa->ep.eModel.rScore)
			admitt=true;
		else
			admitt=false;
	}	
	
	if(admitt){	
		con.at=now;
		flowList.insert(con); //insert flow in list of admitted flows
		aps.pp.numFlows=flowList.size(); //update the number of admitted flows
	}else{
		aps.pp.numRejects++;
	}	

	if(DEBUG) printf("Class %i::OQA (rScore = %lf, rScoreHat = %lf, rScoreHatPlus = %lf) -> Flow at %.2lf %s\n\tNumber admissions = %i\n\tNumber rejections = %i\n\n", aps.dscp, aps.ep.acpoqa->ep.eModel.rScore, aps.ep.acpoqa->ep.eModel.rScoreHat, ars, now, (admitt)?("Admitted"):("Rejected"), aps.pp.numFlows, aps.pp.numRejects);
	if(aps.pp.trace) writeAcTrace(OQA, admitt);
	return(admitt);
}

bool acPolicy::fhbAlgorithm(conId con){
	bool admitt=false; //we are pessimists
	double ebHat=0.0, ebHatMax=0.0, cumDf=0.0, myTmp=0.0, hbTmp=0.0, lgTmp=0.0, sumTmp=0.0, hbArgTmp=0.0;
	int n=aps.pp.numFlows; double epsilon=aps.pp.pLoss; double p=aps.pp.peakRate;
	double now=Scheduler::instance().clock();

	if(n<aps.pp.maxFlows){
		//no congestion as the link is not fully loaded
		if(DEBUG) printf("Class %i::FHB - Charging stage: MaxNumFlows = %i\n", aps.dscp, aps.pp.maxFlows);
		admitt=true;
	}else{
		if(DEBUG) printf("Class %i::FHB - Saturation stage\n", aps.dscp);
		dts=0;
		
		//account for delayed perception of newly admitted flows impact
		if(aps.pp.dipe)	cumDf=calcDipeFactor(); //TODO: DELAYFACTOR ON/OFF INCOMPLETE
		
		//check for resources
		
		for(int ts=1; ts<=aps.ep.acpfhb->mp.tsm; ts++){
			if(DEBUGG){
				myTmp=estimator->estimate(ts, cumDf);
				lgTmp=log(1/epsilon);
				sumTmp=n*p*p;
				hbArgTmp=0.5*lgTmp*sumTmp;
				hbTmp=sqrt(hbArgTmp);
				printf("TS = %i, MY = %.3lf, E = %.5lf, lgTmp = %.3lf, sumTmp = %.3lf, hrArgTmp = %.3lf, hbTmp = %.6lf\n", ts, myTmp, epsilon, lgTmp, sumTmp, hbArgTmp, hbTmp);
				//ebHat=myTmp+hbTmp;
			}
			ebHat=estimator->estimate(ts, cumDf)+sqrt(0.5*log(1/epsilon)*n*p*p); //calculate the Effective Bandwith with risk pLoss
			if(ebHatMax<ebHat){
				ebHatMax=ebHat;
				dts=ts;
			}	
			if(ebHatMax+p>aps.pp.capacity){ //TODO: What is the difference between greater and greaterEqual
				aps.pp.numRejects++;
				admitt=false;
				break;				
			}
			admitt=true;
		}
		//if(DEBUGGG) estimator->getMeter()->dumpSample();
	}
	
	if(admitt){
		con.at=now;
		flowList.insert(con); //insert flow in list of admitted flows
		aps.pp.numFlows=flowList.size(); //update the number of admitted flows
	}

	if(DEBUG) printf("Class %i::FHB (pLoss = %.5lf, capacity = %.2lf, ebHatMax = %.3lf) -> Flow at %.2lf %s\n\tDTS = %i \
				\n\tNumber admissions = %i\n\tNumber rejections = %i\n\n", aps.dscp, aps.pp.pLoss, aps.pp.capacity, ebHatMax, \
				now, (admitt)?("Admitted"):("Rejected"), dts, aps.pp.numFlows, aps.pp.numRejects);

	aps.ep.acpfhb->ebHat=ebHatMax;
	if(aps.pp.trace) writeAcTrace(FHB, admitt);
	return(admitt);
}

double acPolicy::calcDipeFactor(){
	double df=0.0, cmdf=0.0, delta=0.0; //delay factor, cumulative delay factor, time_delta
	double now=Scheduler::instance().clock();
	double wlen=aps.ep.acpskde->mp.wlen; //all flows beyond the window length are ignored
	double theta=aps.pp.theta;
	
	
	//go through the list of admitted flows
	it=flowList.end();
	for(it--; it!=flowList.begin(); it--){
		delta=(now-it->at);
		if(delta>wlen) break; // we dont consider flows older then now-wlen
		df=exp(-theta*delta);
		cmdf+=df;
		if(DEBUGG) printf("AT flow(%i) = %lf, delta=%lf, DF = %lf, CMDF = %lf\n", it->fid, it->at, delta, df, cmdf);
	}
	return(cmdf);
}

void acPolicy::flowInActivityCheck(){
	int i=0, c=0;
	double now=Scheduler::instance().clock();

	it=flowList.begin();
	for(i=0; (unsigned)i<flowList.size(); i++){
		if((now-(it->timeStamp))>=aps.pp.flowTimeout){
			tmpIt=it++;
			flowList.erase(tmpIt);
			c++;
			//printf("Flow id = %i Erased IDs = %i\n", it->fid, c);
		}else{
			it++;
		}	
	}
	
	aps.pp.numFlows=flowList.size(); //update the number of admitted flows
	printf("\n\n%.2lf: Policy Flow Activity Monitor\nExpired admissions %i\nTotal Number of Admissions %i\nNext revision at %.2lf\n", now, c, aps.pp.numFlows, now+aps.pp.checkPeriod);
}

void acPolicy::stopSource(Packet *pkt){
	Agent *agent;
	hdr_cmn* cmh=hdr_cmn::access(pkt);
	agent=(Agent *)cmh->srcPointer();
	agent->close();
}

void acPolicy::dropEventHandler(int fid){
	//this packet belongs to an admitted flow but has been dropped by the queue
	if(aps.pp.flowMon && aps.pp.flowMonMinID <= fid && fid <= aps.pp.flowMonMaxID){
		printf("FID = %i\n", fid);
			lossMon[fid].pop_back();
			lossMon[fid].push_back('-'); //packet has been lost
		//vit=lossMon[fid].end();
		//(*vit)=2;
	}
	estimator->dropEventHandler();
}

double acPolicy::getCheckPeriod(){
	return(aps.pp.checkPeriod);
}

int acPolicy::getDscp(){
	return(aps.dscp);
}

double acPolicy::getCapacity(){
	return(aps.pp.capacity);
}

double acPolicy::getPeakRate(){
	return(aps.pp.peakRate);
}

double acPolicy::getBufferSize(){
	return(aps.pp.bufferSize);
}

int acPolicy::getNumActiveFlows(){
	return(aps.pp.numFlows);
}

int acPolicy::getMaxNumFlows(){
	return(aps.pp.maxFlows);
}

admissionControlType acPolicy::getAdcType(){
	return(aps.adcType);
}

void acPolicy::printAdcSetup(){
	printf("Selected MBAC algorithm: adcType = ");
	switch(aps.adcType){
		case SSUM: printf("SSUM\n"); break;
		case SKDE: printf("SKDE\n"); break;
		case FHB: printf("FHB\n"); break;
		case OQA: printf("OQA\n"); break;
		default: printf("ERROR: Unknown adcType\n");
	}
	printf("\tPolicy Module:\n\t\tpolType = ");
	switch(aps.pp.polType){
		case ACCEPT: 	printf("ACCEPT\n"); break;
		case REPLACE: printf("REPLACE\n"); break;
		default: printf("ERROR: Unknown polType\n");
	}
	printf("\t\tNetwork & Source settings:\n\t\t\tcapacity = %.1f bps\n\t\t\tbufferSize = %.1f bit\n\t\t\tpeakRate = %.1f bps\n\t\t\tpLoss = %f\n", \
			 aps.pp.capacity, aps.pp.bufferSize, aps.pp.peakRate, aps.pp.pLoss);
	printf("\t\tFlow managment:\n\t\t\tcheckPeriod = %.2f\n\t\t\tflowTimerout = %.2f\n", \
			aps.pp.checkPeriod, aps.pp.flowTimeout);
	printf("\t\tTracing: %s\n", (aps.pp.trace)?("Enabled"):("Disabled"));
	printf("\t\tDIPE: %s\n\t\t\tDIPE Factor: %lf\n", (aps.pp.dipe)?("Enabled"):("Disabled"), \
			(aps.pp.dipe)?(aps.pp.theta):(0.0));
	printf("\t\tFLOWMONE: %s\n\t\t\tMin Flow ID: %lf\n\t\t\tMax Flow ID: %lf\n\t\t\tDump Record at: %lf\n", (aps.pp.flowMon)?("Enabled"):("Disabled"), \
			(aps.pp.flowMon)?(aps.pp.flowMonMinID):(0.0), (aps.pp.flowMon)?(aps.pp.flowMonMaxID):(0.0), (aps.pp.flowMon)?(aps.pp.dtime):(0.0));

	if(estimator!=NULL) estimator->printConfig();
}

void acPolicy::writeAcTrace(admissionControlType act, bool admitt){
	snprintf(fileName, 128, "acTrace-%i.tr", getDscp());
	if((tf=fopen(fileName, "a"))==NULL){
		perror("acPolicy::acPolicy:");
		return; 
	}else{
		switch(act){
			case SSUM:	fprintf(tf, "1 %.3lf %i %i\n", Scheduler::instance().clock(), (admitt)?(1):(0), aps.pp.numFlows);
									break;
			case SKDE:	fprintf(tf, "1 %.3lf %i %i %i\n", Scheduler::instance().clock(), (admitt)?(1):(0), dts, aps.pp.numFlows);
									break;
			case FHB:		fprintf(tf, "1 %.3lf %i %.2lf %i %i\n", Scheduler::instance().clock(), (admitt)?(1):(0), aps.ep.acpfhb->ebHat, dts, aps.pp.numFlows);
									break;
			case OQA:		fprintf(tf, "1 %.3lf %i %.2lf %.2lf %i\n", Scheduler::instance().clock(), (admitt)?(1):(0), aps.ep.acpoqa->ep.eModel.rScore, aps.ep.acpoqa->ep.eModel.rScoreHat, aps.pp.numFlows);
									break;
			default:	printf("ERROR: Unknown admission control type, no trace written\n");
									break;
		}							
		fclose(tf);
	}
}

void acPolicy::dumpLossMonitor(){
	FILE *ltf;
	snprintf(fileName, 128, "acTrace-%i-LossMon.tr", getDscp());
	if((ltf=fopen(fileName, "a"))==NULL){
		perror("acPolicy::acPolicy:");
		return; 
	}else{
		for(int fid=0; fid<MAXPERFLOWSTATS; fid++){
			if(lossMon[fid].size()>1){
				for(unsigned ii=0; ii < lossMon[fid].size(); ii++){
					fprintf(ltf, "%c,", lossMon[fid][ii]);
				}
				fprintf(ltf, "\n");
			}
		}
	}
	fclose(ltf);
}