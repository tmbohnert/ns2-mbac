//
// C++ Implementation: emodel
//
// Description: 
//
//
// Author: Thomas Michael Bohnert <tmb@nginet.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "emodel.h"
#include <stdio.h>
#include <math.h>

EModel::EModel(int dataLen, double frlen){
	if(dataLen>MAXDATALEN){
		dataLen=MAXDATALEN;
		printf("WARNING: Data record length has been set to MAXDATALEN");
	}
	eModel.initDefaults(dataLen);
	eModel.frlen=frlen;
}

void EModel::reset(int dataLen, double frlen){
	if(dataLen>MAXDATALEN){
		dataLen=MAXDATALEN;
		printf("WARNING: Data record length has been set to MAXDATALEN");
	}
	eModel.reset(dataLen);
	eModel.frlen=frlen;
}
	
void EModel::meterUpdate(double rcv){
	eModel.pkts++; //total packets seen
	if(rcv==0){
		double pktRcvSt, pktConRcv, lossRatio=0.0;
		double ie=0.0, mieHat=0.0;
		bool sc=false; //state changed?

		eModel.loss++; // packet lost, update loss counter
		pktConRcv=eModel.pkts-eModel.pktRefLe; //consecutively received packets since last loss event
		pktRcvSt=eModel.pkts-eModel.pktRefSt; //packets received since previous state transition

		//packet drop event, update EModel
		if(eModel.state){ //loss burst state
			if(pktConRcv>=16){
				// state has changed meanwhile, update model
				if(pktRcvSt>0)
					lossRatio=(eModel.loss-1)/(pktRcvSt-pktConRcv); //loss rate at the state transition
				ie=lossImpairFactor(lossRatio, 0); 
				updateIeRecord(ie, (pktRcvSt-pktConRcv)*eModel.frlen, eModel.state);
				printf("\nLoss Burst -> Gap %lf %lf %lf\n", lossRatio, ie, (pktRcvSt-pktConRcv)*eModel.frlen);
				//init loss gap state
				eModel.state=0;
				eModel.loss=1; // reset loss counter
				eModel.pktRefSt=eModel.pkts-pktConRcv; //absolute number of packets sent by this state transition
				sc=true;
			}	
		}else{
			if(pktConRcv<=16){ //TODO: Activity factor configurable or estimated
				// state has changed meanwhile, update model
				/* an isolated packet packet drop occured within a distance of less than 16 packets */
				if(pktRcvSt>0)
					lossRatio=((pktRcvSt-pktConRcv)>0)?((eModel.loss-1)/(pktRcvSt-pktConRcv)):(0); //loss rate at the state transition
				ie=lossImpairFactor(lossRatio, 0);
				updateIeRecord(ie, (pktRcvSt-pktConRcv)*eModel.frlen, eModel.state);
				printf("Loss Gap -> Burst, %lf %lf %lf\n", lossRatio, ie, (pktRcvSt-pktConRcv)*eModel.frlen);
				//init loss burst state
				eModel.state=1;
				eModel.loss=1; // reset loss counter
				eModel.pktRefSt=eModel.pkts-pktConRcv; //absolute number of packets sent by this state transition
				sc=true;
			}
		}
		//update packet reference
		eModel.pktRefLe=eModel.pkts;
	}
}

void EModel::updateModel(){
	double pktRcvSt, pktConRcv, lossRatio=0.0;
	double ie=0.0, mieHat=0.0;
	bool sc=false; //state changed?
	
	pktConRcv=eModel.pkts-eModel.pktRefLe; //consecutively received packets since last loss event
	pktRcvSt=eModel.pkts-eModel.pktRefSt; //packets received since previous state transition
		
	//update EModel in case there was no packet loss event recently
	if(eModel.state){ //loss burst state
		if(pktConRcv>=16){
			if(pktRcvSt>0)
				lossRatio=(eModel.loss)/(pktRcvSt-pktConRcv); //loss rate at the state transition
			ie=lossImpairFactor(lossRatio, 0);
			updateIeRecord(ie, (pktRcvSt-pktConRcv)*eModel.frlen, eModel.state);
			printf("\nLoss Burst -> Gap %lf %lf %lf\n", lossRatio, ie, (pktRcvSt-pktConRcv)*eModel.frlen);
			//init loss gap state
			eModel.state=0;
			eModel.loss=0; // reset loss counter
			eModel.pktRefSt=eModel.pkts-pktConRcv; //absolute number of packets sent by this state transition
			sc=true;
		}else{
			//no state transition, we are somewhere withing a loss burst
			if(pktRcvSt>0){
				lossRatio=(eModel.loss)/(pktRcvSt); //loss rate since last state transition
				ie=lossImpairFactor(lossRatio, 0);
				updateIeRecord(ie, pktRcvSt*eModel.frlen, eModel.state);
			}
			sc=false;
		}
	}
	if(!eModel.state){ //loss gap state
		if(sc){ //if a state transition has been detected before, we have account for the gap since this state transition
			pktConRcv=eModel.pkts-eModel.pktRefSt;
			pktRcvSt=pktConRcv; //packets received since previous state transition
		}else{
			pktConRcv=eModel.pkts-eModel.pktRefLe; //consecutively received packets since last loss event
			//printf("Consecutively received packets since last loss event: %lf\n", pktConRcv);
			pktRcvSt=eModel.pkts-eModel.pktRefSt; //packets received since previous state transition
		}
		if(pktRcvSt>0){
			lossRatio=(eModel.loss)/(pktRcvSt); //loss rate since last state transition
			ie=lossImpairFactor(lossRatio, 0);
			updateIeRecord(ie, pktRcvSt*eModel.frlen, eModel.state);
		}
	}	
}

double EModel::lossImpairFactor(double lossRatio, int model){
	double x=lossRatio*100;
	switch(model){
		case 0: //polynominal fit, 4th order.
		{
			if(lossRatio<=8) //non-linear part
				return(-0.00094355*x+0.19543*x-1.4758*x+5.1598*x-0.8902*x);
			else
				return(1.286*x-1.786);
			break;
		}
		default: printf("Could'nt find model for EModel Ie fiting\n");
	}
	return(-1);
}

double EModel::updateIeRecord(double ie, double t, int state){
	// add the latest estimate and update the head pointer
	if(state==1){
		eModel.ieRec[eModel.ieRecHead].ieb=ie;
		eModel.ieRec[eModel.ieRecHead].ieg=0.0;
		eModel.ieRec[eModel.ieRecHead].blen=t;
		eModel.ieRec[eModel.ieRecHead].glen=0.0;
		eModel.ieRec[eModel.ieRecHead].state=state;
	}else{
		eModel.ieRec[eModel.ieRecHead].ieg=ie;
		eModel.ieRec[eModel.ieRecHead].ieb=0.0;
		eModel.ieRec[eModel.ieRecHead].glen=t;
		eModel.ieRec[eModel.ieRecHead].blen=0.0;
		eModel.ieRec[eModel.ieRecHead].state=state;
	}
	eModel.ieRecHead++;
}

double EModel::calcMeanIe(){
	double ieb=0.0; double ieg=0.0;
	double blen=0.0; double glen=0.0;
	double ieTwo=0.0;
	int bc=0; int gc=0;

	//compute the averages, ieg, ieb, blen and glen
	for(int idx=eModel.ieRecHead-1; idx>=0; idx--){
		if(eModel.ieRec[idx].state==1){ //burst state
			ieb+=eModel.ieRec[idx].ieb;
			blen+=eModel.ieRec[idx].blen;
			bc++;
		}
		if(eModel.ieRec[idx].state==0){
			ieg+=eModel.ieRec[idx].ieg;
			glen+=eModel.ieRec[idx].glen;
			gc++;
		}
	}
 	if(bc>0){
		eModel.ieBHat=ieb/bc;
 		eModel.bLenHat=blen/bc;
	}
	if(gc>0){	
		eModel.ieGHat=ieg/gc;
 		eModel.gLenHat=glen/gc;
	}

		//calculate mIeHat as in [Raake06] equation 19.
	ieTwo=eModel.ieGHat*(1-exp(-(eModel.gLenHat/22)))+eModel.ieBHat*(1-exp(-(eModel.bLenHat/9)))*exp(-(eModel.gLenHat/22));
	eModel.mieHat=1/(eModel.bLenHat+eModel.gLenHat)*(eModel.ieBHat*eModel.bLenHat+eModel.ieGHat*eModel.gLenHat+9*(eModel.ieBHat-ieTwo)*(exp(-(eModel.bLenHat/9))-1)-22*(eModel.ieBHat-(eModel.ieBHat-ieTwo)*exp(-(eModel.bLenHat/9))-eModel.ieGHat)*(exp(-(eModel.gLenHat/22)-1)));
	if(eModel.mieHat<0.0)
		eModel.mieHat=0.0;
	
	printf("ieBHat=%lf, blenHat=%lf, ieGHat=%lf, glenHat=%lf, mieHat=%lf\n", eModel.ieBHat, eModel.bLenHat, eModel.ieGHat, eModel.gLenHat, eModel.mieHat);

	return(eModel.mieHat);
}

double EModel::estimate(){
	/* IeHat updates happen only on state transitions. If a state persists for a long time, it is not reflected in the current R score*/
	printf("Account for the period between flow end and the last loss event and calculate the final RScore\n");
	updateModel();
	calcMeanIe();
	return(eModel.mieHat);
}

