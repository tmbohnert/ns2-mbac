//
// C++ Implementation: dsAdc
//
// Description: bothom:mbac
//
//
// Author: Thomas Bohnert <tbohnert@dei.uc.pt>, bothom:mbac (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "dsAdc.h"
#include "packet.h"
#include "acEstimator.h"
#include "acDefinitions.h"
#include "dsEdge.h"

dsAdc::dsAdc():tcl(Tcl::instance()){
	adcPoolIndex=0;
	adcDebug=true;
}	

dsAdc::~dsAdc(){
}

void dsAdc::addAdcEntry(int argc, const char*const* argv){
	if (adcPoolIndex==MAX_ADCS)
		printf("ERROR: AdcPool size limit exceeded.\n");
	else {
		//create a new AC setup
		adcParameterSet ps;
		ps.init(); //reset all AC paramters to default (SSUM) configuration
		adcPool[adcPoolIndex].dscp=atoi(argv[2]);
		
/* --------  SIMPLE SUM (SSUM) algorithm -------- */
    if(strcmp(argv[3], "SSUM") == 0){
			ps.dscp=atoi(argv[2]);
			ps.adcType=SSUM; 
			ps.pp.polType=REPLACE; //has no meaning here since implicit for SSUM
			int idx=4;
			//SSUM is purely implemented in the policy class
			if(strcmp(argv[idx], "DEFAULT")!=0){ //explicit configuration
				ps.pp.capacity=atof(argv[idx]);
				ps.pp.peakRate=atof(argv[++idx]);
				if(strcmp(argv[++idx], "TRE")!=0)
					ps.pp.trace=false;
				else
					ps.pp.trace=true;	
			}	
			ps.pp.maxFlows=(int)(ps.pp.capacity/ps.pp.peakRate);
			ps.ep.estType=NLEST;
		}
		
/* --------  SIMPLE KERNEL DENSITY ESTIMATION (SKDE) algorithm -------- */
		if(strcmp(argv[3], "SKDE") == 0){
			ps.dscp=atoi(argv[2]);
			ps.adcType=SKDE;
			int idx=4;
			//conifgure policy 
			if(strcmp(argv[idx], "DEFAULT")!=0){ //explicit configuration
				ps.pp.polType=configPolicy(argv, idx);
				ps.pp.capacity=atof(argv[++idx]);
				ps.pp.bufferSize=atoi(argv[++idx]);
				ps.pp.peakRate=atof(argv[++idx]);
				ps.pp.pLoss=atof(argv[++idx]);
				if(strcmp(argv[++idx], "TRE")!=0)
					ps.pp.trace=false;
				else
					ps.pp.trace=true;	
				//TODO: Make the remainng policy parameters configurable
			}
			ps.pp.maxFlows=(int)(ps.pp.capacity/ps.pp.peakRate);

			//configure estimator
			ps.ep.estType=SKDEEST;
			ps.ep.acpskde=new acParameterSKDE;
			ps.ep.acpskde->init(); //default values
			if(strcmp(argv[++idx], "DEFAULT")!=0){ //explicit configuration
				ps.ep.acpskde->ep.h=atof(argv[idx]);
				ps.ep.acpskde->ep.j=atoi(argv[++idx]);
				if(strcmp(argv[++idx], "IE")!=0) //Interval estimate?
					ps.ep.acpskde->ep.intEst=false;
				else
					ps.ep.acpskde->ep.intEst=true;
				if(strcmp(argv[++idx], "TRE")!=0) //Tracing enabled?
					ps.ep.acpskde->ep.trace=false;
				else
					ps.ep.acpskde->ep.trace=true;
				
				//configure meter
				ps.ep.acpskde->mp.tau=atof(argv[++idx]);
				ps.ep.acpskde->mp.tsm=atoi(argv[++idx]);
				ps.ep.acpskde->mp.wlen=atof(argv[++idx]);
				if(strcmp(argv[++idx], "TRE")!=0)
					ps.ep.acpskde->mp.trace=false;
				else
					ps.ep.acpskde->mp.trace=true;
				
				//check for extension
				while(strcmp(argv[++idx], "END")!=0){
					//Delayed Impact Perception Extesion (DIPE)
					if(strcmp(argv[idx], "DIPE")==0){
						ps.pp.dipe=ON;
						ps.pp.theta=atof(argv[++idx]);
					}
				}
			}
			//set the algorithm's parameters/thresholds according to the set values
			if(ps.ep.acpskde->mp.tsm>MAXTIMESCALES) printf("WARNING: Configured time scales exceed MAXTIMESCALES. Reset to maximum of %i\n", ps.ep.acpskde->mp.tsm=MAXTIMESCALES);
			for(int tsi=0; tsi<ps.ep.acpskde->mp.tsm; tsi++){
				//set the overflow threshold (in Byte)
				ps.ep.acpskde->ep.x[tsi]=((ps.pp.capacity-ps.pp.peakRate)/8)*(ps.ep.acpskde->mp.tau*(tsi+1))+(ps.pp.bufferSize/8);
				//set sample size/window length (memory of the algorithm)
				ps.ep.acpskde->mp.n[tsi]=(int)(ps.ep.acpskde->mp.wlen/(ps.ep.acpskde->mp.tau*(tsi+1)));
			}
		}

/* --------  FLOYDHB (FHB) algorithm -------- */
    if(strcmp(argv[3], "FHB") == 0){
			ps.dscp=atoi(argv[2]);
			ps.adcType=FHB;
			ps.pp.polType=REPLACE; //has no meaning here since implicit for SSUM
			int idx=4;
			//conifgure policy 
			if(strcmp(argv[idx], "DEFAULT")==0){ //explicit configuration
				printf("ERROR: No DEFAULT configuration for the FHB algorithm policy module defined\n");
				return;
			}else{
				ps.pp.polType=configPolicy(argv, idx);
				ps.pp.capacity=atof(argv[++idx]);
				ps.pp.bufferSize=atoi(argv[++idx]); //According to [FLOYD96, Comments on MBAC for Control-Load], this value is always zero
				ps.pp.peakRate=atof(argv[++idx]);
				ps.pp.pLoss=atof(argv[++idx]);
				if(strcmp(argv[++idx], "TRE")!=0)
					ps.pp.trace=false;
				else
					ps.pp.trace=true;
			}
			ps.pp.maxFlows=(int)(ps.pp.capacity/ps.pp.peakRate);

			//configure estimator
			ps.ep.estType=FHBEST;
			ps.ep.acpfhb=new acParameterFHB;
			ps.ep.acpfhb->init(); //default values
			if(strcmp(argv[++idx], "DEFAULT")==0){ //explicit configuration
				printf("ERROR: No DEFAULT configuration for FHB algorithm estimator module defined\n");
				return;
			}else{
				ps.ep.acpfhb->ep.t=atof(argv[idx]);
				if(strcmp(argv[++idx], "TRE")!=0) //tracing enabled?
					ps.ep.acpfhb->ep.trace=false;
				else
					ps.ep.acpfhb->ep.trace=true;
			}

			//configure meter
			ps.ep.acpfhb->mp.tau=atof(argv[++idx]);
			ps.ep.acpfhb->mp.tsm=atoi(argv[++idx]);
			ps.ep.acpfhb->mp.wlen=atof(argv[++idx]);
			if(strcmp(argv[++idx], "TRE")!=0)
				ps.ep.acpfhb->mp.trace=false;
			else
				ps.ep.acpfhb->mp.trace=true;
			
			//check for extension
			while(strcmp(argv[++idx], "END")!=0){
				if(strcmp(argv[idx], "FLOWMONE")==0){
					ps.pp.flowMon=ON;
					ps.pp.flowMonMinID=atoi(argv[++idx]);
					ps.pp.flowMonMaxID=atoi(argv[++idx]);
					ps.pp.dtime=atof(argv[++idx]);
					if(ps.pp.flowMonMinID>ps.pp.flowMonMaxID){
						printf("Min Flow ID larger than Max Flow ID. Set Min ID to MAX ID\n");
						ps.pp.flowMonMinID=ps.pp.flowMonMaxID;
					}
					continue;	
				}
				if(strcmp(argv[idx], "CPE")==0){
					ps.ep.acpfhb->ep.cpe=ON;
					ps.ep.acpfhb->ep.k=atof(argv[++idx]);
					ps.ep.acpfhb->ep.lmbd=atof(argv[++idx]);
					continue;
				}
			printf("ERROR: extenstions not defined for FHB algorithm available. Final \"END\" flag missing?\n");
			}	
			
			//set the algorithm's parameters/thresholds according to the set values
			if(ps.ep.acpfhb->mp.tsm>MAXTIMESCALES) printf("WARNING: Configured time scales exceed MAXTIMESCALES. Reset to maximum of %i\n", ps.ep.acpfhb->mp.tsm=MAXTIMESCALES);
			for(int tsi=0; tsi<ps.ep.acpfhb->mp.tsm; tsi++){
				ps.ep.acpfhb->ep.w[tsi]=1-exp(-(ps.ep.acpfhb->mp.tau*(tsi+1))/ps.ep.acpfhb->ep.t);
			}
		}

/* --------  Objective QoS Assessment (OQA) algorithm -------- */
		if(strcmp(argv[3], "OQA") == 0){
			ps.dscp=atoi(argv[2]);
			ps.adcType=OQA;
			ps.pp.polType=REPLACE;
			int idx=4;
			//configure policy module
			if(strcmp(argv[idx], "DEFAULT")==0){ //explicit configuration
				printf("ERROR: No DEFAULT configuration for OQA algorithm polify module defined\n");
				return;
			}else{
				ps.pp.polType=configPolicy(argv, idx);
				ps.pp.capacity=atof(argv[++idx]);
				ps.pp.bufferSize=atoi(argv[++idx]);
				ps.pp.peakRate=atof(argv[++idx]);
				ps.pp.pLoss=atof(argv[++idx]);
				if(strcmp(argv[++idx], "TRE")!=0)
					ps.pp.trace=false;
				else
					ps.pp.trace=true;
			}
			ps.pp.maxFlows=(int)(ps.pp.capacity/ps.pp.peakRate);

			//configure estimator module
			ps.ep.estType=OQAEST;
			ps.ep.acpoqa=new acParameterOQA;
			ps.ep.acpoqa->init(); //default values
			ps.ep.acpoqa->ep.eModel.rScore=atof(argv[++idx]);
			ps.ep.acpoqa->ep.eModel.frlen=atof(argv[++idx]);
			ps.ep.acpoqa->ep.eModel.pktSize=atof(argv[++idx]);
			ps.ep.acpoqa->ep.eModel.wLen=atof(argv[++idx]);
			if(strcmp(argv[++idx], "TRE")!=0) //Tracing enabled?
				ps.ep.acpoqa->ep.trace=false;
			else
				ps.ep.acpoqa->ep.trace=true;

			//configure meter module
			// no meter for OQA algorithm required/defined
  	
			//check for extension
			while(strcmp(argv[++idx], "END")!=0){
				if(strcmp(argv[idx], "FLOWMONE")==0){
					ps.pp.flowMon=ON;
					ps.pp.flowMonMinID=atoi(argv[++idx]);
					ps.pp.flowMonMaxID=atoi(argv[++idx]);
					ps.pp.dtime=atof(argv[++idx]);
					if(ps.pp.flowMonMinID>ps.pp.flowMonMaxID){
						printf("Min Flow ID larger than Max Flow ID. Set Min ID to MAX ID\n");
						ps.pp.flowMonMinID=ps.pp.flowMonMaxID;
					}
					continue;
				}
				printf("ERROR: Extension not defined for the OQA algorithm and final \"END\" flag missing!\n");
			}

		//set the algorithm's parameters/thresholds according to the set values
		}

	//create a policy object in order to trigger the complete MBAC algorithm structure
	adcPool[adcPoolIndex].policy=new acPolicy(ps);
	adcPoolIndex++;
	}
}	
		
policyType dsAdc::configPolicy(const char*const* argv, int idx){
	if(strcmp(argv[idx], "ACCEPT")==0 || strcmp(argv[idx], "accept")==0)
		return(ACCEPT);
	if(strcmp(argv[idx], "REPLACE")==0 || strcmp(argv[idx], "replace")==0)
		return(REPLACE);
	if(DEBUG) printf("WARNING: Could not define admission policy! Set back to default: ACCEPT\n");
	return(ACCEPT);
}

bool dsAdc::admissionRequest(Packet *pkt){
	int dscp=0, index=0;
	hdr_ip* iph;
	bool admitt=false;

	//class identification
	iph=hdr_ip::access(pkt);
	dscp=iph->prio();
	//identify the admission control algorithm to apply to this class
	index=lookupAdcPool(dscp);
	
	if(index>=0){
		admitt=(adcPool[index].policy)->admissionRequest(pkt);
	}else{
		if(DEBUG) printf("No ADC defined for this DSCP\n");
		admitt=false;	//dscp does not exist! By default, packet will be dropped
	}
	return(admitt);
}

int dsAdc::lookupAdcPool(int dscp){
	int c=0;
	
	while(c<adcPoolIndex){
		if(adcPool[c].dscp==dscp){ 
			return(c); // DSCP <-> AC mapping found
		}else{
			c++;
		}
	}
	if(DEBUG) printf("ERROR: No ADC for DSCP:%i configured\n", dscp);
	return(-1); //no mapping found, return invalid index such that the packet will be dropped
}

void dsAdc::printAdcSetup(){
	for(int c=0; c<adcPoolIndex; c++){
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\nADC SETUP FOR DSCP:%i\n", adcPool[c].dscp);
		adcPool[c].policy->printAdcSetup();
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
	}
}

void dsAdc::pktDropEvent(int dscp, int fid){
	//identify the admission control algorithm to apply to this class
	int index=lookupAdcPool(dscp);
	if(index>=0)
		(adcPool[index].policy)->dropEventHandler(fid);
	else
		if(DEBUG) printf("No ADC defined for this DSCP\n");
}

