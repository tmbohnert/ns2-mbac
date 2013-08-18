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

dsAdc::dsAdc():tcl(Tcl::instance()){
	adcPoolIndex=0;
	adcDebug=false;
}	

dsAdc::~dsAdc(){
}

void dsAdc::addAdcEntry(int argc, const char*const* argv){
	if (adcPoolIndex==MAX_ADCS)
		printf("ERROR: AdcPool size limit exceeded.\n");
	else{
		//create a new AC setup
		adcParameterSet ps;
		ps.init(); //reset all AC paramters to default (SSUM) configuration
		adcPool[adcPoolIndex].dscp=atoi(argv[2]);
		
		//configure the SSUM algorithm
    if(strcmp(argv[3], "SSUM") == 0){
			//TODO: Check for a min number of parameters to avoid errors
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
		
		//configure the SKDE algorithm
		if(strcmp(argv[3], "SKDE") == 0){
			//TODO: Check for a min number of parameters to avoid errors
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
			}

			//set global parameters/thresholds
			//ps.ep.acpskde->ep.x=new double[tsm];
			if(ps.ep.acpskde->mp.tsm>MAXTIMESCALES) printf("WARNING: Configured time scales exceed MAXTIMESCALES. Reset to maximum of %i\n", ps.ep.acpskde->mp.tsm=MAXTIMESCALES);
			for(int tsi=0; tsi<ps.ep.acpskde->mp.tsm; tsi++){
				//set the overflow threshold (in Byte)
				ps.ep.acpskde->ep.x[tsi]=((ps.pp.capacity-ps.pp.peakRate)/8)*(ps.ep.acpskde->mp.tau*(tsi+1))+(ps.pp.bufferSize/8);
				//set sample size/window length (memory of the algorithm)
				ps.ep.acpskde->mp.n[tsi]=(int)(ps.ep.acpskde->mp.wlen/(ps.ep.acpskde->mp.tau*(tsi+1)));
			}
		}
				
		adcPool[adcPoolIndex].policy=new acPolicy(ps); //generate a new policy object
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
	
	if(index>=0)
		admitt=(adcPool[index].policy)->admissionRequest(pkt);
	else
		admitt=false;	//dscp does not exist! By default, packet will be dropped
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
	printf("ERROR: No ADC for DSCP:%i configured\n", dscp);
	return(-1); //no mapping found, return invalid index such that the packet will be dropped
}

void dsAdc::printAdcSetup(){
	for(int c=0; c<adcPoolIndex; c++){
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\nADC SETUP FOR DSCP:%i\n", adcPool[c].dscp);
		adcPool[c].policy->printAdcSetup();
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
	}
}
