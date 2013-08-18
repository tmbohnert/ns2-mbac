//
// C++ Interface: acDefinitions
//
// Description: 
//
//
// Author: Thomas Bohnert <tbohnert@dei.uc.pt>, bothom:mbac (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ACDEFINITIONS_H
#define ACDEFINITIONS_H

#include "packet.h"
#include "ip.h"
#include "agent.h"
#include <math.h>
#include "timer-handler.h"

#define MAX(a, b) (a>b?a:b)

#define MAXPERFLOWSTATS 10000
#define DEFAULT_FLOW_TIMEOUT 50.0
#define DEFAULT_CHECK_PERIOD 10.0
#define MAXTIMESCALES 10
#define ON true;
#define OFF false;



//Three debug/error levels
//1st, severe errors are printed to stdout in any case
//2nd, DEBUG is for general information, sort of verbose mode reporting AC actions
//3rd, DEBUGG is defined for code debugging. This information are only printed during developing phases
//4th, DEBUGGG is for other code debugging operations (mainly file writting)
#define DEBUG 1
#define DEBUGG 0
#define DEBUGGG 0


class acPolicy;

//implemented admission control algorithms
enum admissionControlType{SSUM, SKDE, FHB, OQA, EXM}; //SimpleSum, SimpleKernelDenstiyEstimator, FloydHoeffdingBounds, ObjectiveQoSAssessment, Example
//implmented policiy types
enum policyType {ACCEPT, REPLACE};
//implemented estimator type
enum estimatorType {NLEST, SKDEEST, FHBEST, OQAEST, EXMEST}; //Null-Estimator, Simple-Kernel-Denstiy-Estimator, Hoeffing-Bound-Estimator, ObjectiveQoSAssessment-Estimator, Example-Estimator

enum unitType {BYTE, PKT};

//struct PoolEntry
struct adcPoolEntry {
	int dscp; //dscp mapping
	acPolicy *policy; //AC policy
};

class acOQAEstimator;

class EModelTimer : public TimerHandler{
protected:
	acOQAEstimator *est;
	double to; //timeout
public:
	EModelTimer(acOQAEstimator *e, double timeOut) : TimerHandler(), est(e), to(timeOut){}
	void expire(Event *);
};

struct eModelParam{
	bool state;	//0 = gap, 1 = burst
	double rScore; //minimum quality level
	double rScoreHat; //estimated rScore
	double frlen; 	//voip frame length in sec.
	double pktSize;	//packet size
	long pkts; //total received packets
	int loss;
	double pktRefSt;	//number of packet triggering a state transition
	double pktRefLe;	//number of last lost packet (loss event)
	double tRef; double tple; //time of previous state transition and loss event
	int maxDrops; //maximum number of drops between two score updates.
	double wLen;	//observation window of Ie
	struct ieRecord{
		double pkts; //total packets seen
		double pktsLost; //packets lost
		double lr; 	//loss ratio
		double lrPlus; //predicted loss ratio for the aggregate plus one flow (admission prediction)
		double ie;	//Ie without gap/loss association for the model presented in [BohMon06, CCNC06]
		double iePlus; 
		double t;		//time
		int state; //state of the four-state loss model
		double ieg; //next four variables are to calculate average integral quality, see [Clark01, Raake06]
		double ieb;
		double glen;
		double blen;
	} *ieRec;
	
	int ieRecHead; int ieRecHeadTmp; int ieRecTotLen;
	double ieBHat; double ieBHatPlus; double bLenHat; //mean burst Ie and burst length
	double ieGHat; double ieGHatPlus; double gLenHat; //mean gap Ie and gap length
	double mieHat; //current mean Ie
	double mieHatPlus;

	double to; //time out for periodic rScore updates
	EModelTimer *emt; //timer for periodic mIe updates. Asures that long loss gaps/bursts do not delay RScore updates

	void inline initIeRec(){
			ieRecTotLen=(int)(wLen*100);
			ieRec=new ieRecord[ieRecTotLen];
			ieRecHead=0;
			for(int i=0; i<ieRecTotLen-1; i++){
				ieRec[i].pkts=0;
				ieRec[i].pktsLost=0;
				ieRec[i].lr=0.0;
				ieRec[i].lrPlus=0.0;
				ieRec[i].ie=0.0;
				ieRec[i].iePlus=0.0;
				ieRec[i].t=0.0;
				ieRec[i].state=2; //invalid state
				ieRec[i].ieg=0.0;
				ieRec[i].ieb=0.0;
				ieRec[i].glen=0.0;
				ieRec[i].blen=0.0;
			}
			to=5.0;
			emt=NULL;
	};
};

/* --------  SIMPLE KERNEL DENSITY ESTIMATION (SKDE) algorithm -------- */
struct acParameterSKDE {
	struct skdeMeterParam{
		double tau; //sampling period
		int tsm; //tau scaling max value
		double wlen;	//sample length (measurement window, memory of the algorithm)
		int n[MAXTIMESCALES]; //sample sizes
		bool trace;
	} mp;
	
	struct skdeEstParam{
		double h; //Kernel bandwidth, if set to 0, use Gaussian Reference Bandwidth, for any other, compute from actual data
		int j; //Plug-in bandwidth step width
		double x[MAXTIMESCALES]; //set x if F(x) shall be recursively/continously estimated
		bool trace;
		bool intEst; //Interval estimate?
	} ep;

	inline void init(){
		mp.tau=0.02; /*10ms*/
		mp.tsm=5; /*0.02, 0.04, 0.06, 0.08, 0.10 */
		mp.wlen=10.0; /*10s window*/
		for(int tsi=0; tsi<mp.tsm; tsi++)
			mp.n[tsi]=(int)(mp.wlen/(mp.tau*(tsi+1)));
		mp.trace=false;
		ep.h=1.0;
		ep.j=4; //default recommended in "Bandwith Selection for Non-parametric Distribution Estimation" Hansen, 2004
		for(int tsi=0; tsi<mp.tsm; tsi++) ep.x[tsi]=0.0;
		ep.intEst=false;
		ep.trace=false;
	}
};

/* --------  FLOYDHB (FHB) algorithm -------- */
struct acParameterFHB {
	double ebHat; //latest estimated Effective Bandwidth

	struct fhbMeterParam{
		double tau; //sampling period
		int tsm; //tau scaling max value
		double wlen;	//sample length (measurement window, memory of the algorithm)
		int n[MAXTIMESCALES]; //sample sizes
		bool trace;
	} mp;
	
	struct fhbEstParam{
		double t;
		 /* EWMA time constant, That is, assume that the class arrival rate changes abruptly
			from 0 to 1, and that the arrival rate mains at the value 1. After 6 seconds, the
			estimated verage arrival rate will have reached 63% of the new arrival rate value of 1.
		 */
		double w[MAXTIMESCALES]; //respective weights for all time scales
		bool trace;

		//Change Point Extension CPE
		bool cpe;			//CP enabled if set true
		bool initCpe[MAXTIMESCALES]; //CP is initiated if set "true"
		double muHat[MAXTIMESCALES], sigHat[MAXTIMESCALES], acfOneHat[MAXTIMESCALES], ewmaHat[MAXTIMESCALES];
		double ewmaHighHat[MAXTIMESCALES], ewmaLowHat[MAXTIMESCALES];
		double ewma[MAXTIMESCALES];
		double k;
		double lmbd;
	} ep;

	inline void init(){
		ebHat=0.0;
		mp.tau=0.02; /*10ms*/
		mp.tsm=5; /*0.02, 0.04, 0.06, 0.08, 0.10 */
		mp.wlen=10.0; /*10s window*/
		for(int tsi=0; tsi<mp.tsm; tsi++)
			mp.n[tsi]=(int)(mp.wlen/(mp.tau*(tsi+1)));
		mp.trace=false;
		ep.t=1.0;
		for(int tsi=0; tsi<mp.tsm; tsi++){
			ep.w[tsi]=1-exp(-(mp.tau*(tsi+1))/ep.t);
		}	
		ep.trace=false;

		//CP extension
		ep.cpe=false;
		for(int tsi=0; tsi<mp.tsm; tsi++){
			ep.initCpe[tsi]=false;
			ep.ewma[tsi]=ep.muHat[tsi]=ep.sigHat[tsi]=ep.acfOneHat[tsi]=ep.ewmaHat[tsi]=0.0;
			ep.ewmaHighHat[tsi]=ep.ewmaLowHat[tsi]=0.0;				
		}
		ep.k=ep.lmbd=0.0;
	}
};

/* --------  Objecive QoS Assessment (OQA) algorithm -------- */
struct acParameterOQA {

	//no meter for OQA algorithm required
	struct oqaMeterParam{
	} mp;
	
	struct oqaEstParam{
		eModelParam eModel;
 		bool trace;
	} ep;

	inline void init(){
		ep.eModel.state=0; //loss gap
		ep.eModel.rScore=85.0; //default target quality
		ep.eModel.rScoreHat=100.0; //default estimated/measured quality
		ep.eModel.frlen=0.02; //20ms voip frames per packet
		ep.eModel.loss=0;
		ep.eModel.pktRefSt=0;
		ep.eModel.pktRefLe=0;
		ep.eModel.wLen=600;
		ep.eModel.initIeRec();
		ep.trace=false;
	}
};

//struct adcParameterSettings. Defines common parameters for all MBACs, in particular for the general policy object
struct adcParameterSet {
	int dscp;
	//admission control type
	admissionControlType adcType;
	
	//policy settings
	struct policyParameter{
		policyType polType;
		//link specs
		double capacity; //in bit/sec
		double bufferSize; //in bits!
		//flow spec	
		double peakRate; //in bit/sec
		//QoS requirements
		double pLoss; //max overflow probability
		//current number of admitted flows, number of rejections and flow managment
		int numFlows; int numRejects; int maxFlows; //max admissible number for SSUM algorithm
		double flowTimeout, checkPeriod;

		//DIPE, Delayed Impact Perception Extension
		bool dipe;
		double theta; //DIPE wheight factor 
		bool trace;

		//Flow Monitoring Extension (FLOWMONE)
		bool flowMon;
		int flowMonMinID;
		int flowMonMaxID;
		double dtime; //when to dump the recorded flow trace (absolute simulation time)

		//Prediction Extension (PREDE)
		//indicates if the QoS parameter is an estimate of the aggregate or a prediction
		//of the aggregate plus one (to be addmitted) flow
		bool prede;
	} pp;
	
	//estimator settings
	struct estimatorParameter{
		estimatorType estType;
		acParameterSKDE *acpskde;
		acParameterFHB *acpfhb;
		acParameterOQA *acpoqa;
 		//acParameterEXM *acpexm;
	} ep;
	
	//save init routine for default settings, default AC is SSUM
	inline void init(){
		dscp=0; adcType=SSUM;
		pp.polType=ACCEPT; //irrelevant for SSUM, default for all MBACs
		pp.capacity=1e6;/*1Mbit*/ pp.bufferSize=1.5e3; /*bit*/ pp.peakRate=8.16e4;/*(G.711+RTP/UDP/IP/ETH)*/ pp.pLoss=10e-5;
		pp.numFlows=0; 	pp.numRejects=0; pp.maxFlows=1; pp.flowTimeout=DEFAULT_FLOW_TIMEOUT; pp.checkPeriod=DEFAULT_CHECK_PERIOD; 
		pp.trace=false;
		pp.dipe=OFF;
		pp.theta=1.0;
		pp.flowMon=OFF;
		pp.prede=OFF;
		ep.estType=NLEST; ep.acpskde=NULL; ep.acpfhb=NULL;
	}
};

//Connection (flow) identifier
struct conId{
	ns_addr_t	src;
	ns_addr_t	dst;
	int	prio;	//DSCP of this flow
	int fid;
	double hash;	//Unique flow identifier
	double timeStamp;	//time stamp for Black- and WhiteList expiration
	long packetSize;	//some ADC algorithms need this info to calculate the average arrival rate per class
	Agent *agent;	//sender reference for flow tear down
	double at;	//time of admission
	
	void genConHash(void){
		//UPDATE hash=(src.addr_+src.port_+dst.addr_+dst.port_)<<(src.port_+dst.port_*prio);
		hash=fid;
	}
};

//Function object to compare objects in a STL-Set container
class keyCmp{
public:
	bool operator()(const conId &x, const conId &y){
		return(x.hash < y.hash);
	}
};

// bytes count from startTime until ...
struct byteCount{
  long bytes;
  double startTime;
};

# endif
