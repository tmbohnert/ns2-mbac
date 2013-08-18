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

#define MAX(a, b) (a>b?a:b)

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
#define DEBUG 0
#define DEBUGG 0
#define DEBUGGG 0


class acPolicy;

//implemented admission control algorithms
enum admissionControlType{SSUM, SKDE}; //SimpleSum, SimpleKernelDenstiyEstimator, AggregateTrafficEnvelopes
//implmented policiy types
enum policyType {ACCEPT, REPLACE};
//implemented estimator types
enum estimatorType {NLEST, SKDEEST}; //Null-Estimator, Simple-Kernel-Denstiy-Estimator

enum unitType {BYTE, PKT};

//struct PoolEntry
struct adcPoolEntry {
	int dscp; //dscp mapping
	acPolicy *policy; //AC policy
};

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
			mp.n[tsi]=(int)(mp.wlen/(mp.tau*tsi+1));
		mp.trace=false;
		ep.h=1.0;
		ep.j=4; //default recommended in "Bandwith Selection for Non-parametric Distribution Estimation" Hansen, 2004
		for(int tsi=0; tsi<MAXTIMESCALES; tsi++) ep.x[tsi]=0.0;
		ep.intEst=false;
		ep.trace=false;
	}
};

//struct adcParameterSettings
struct adcParameterSet {
	int dscp;
	//admission control type
	admissionControlType adcType;
	
	//policy settings
	struct policyParameter{
		policyType polType;
		//link specs
		double capacity;
		double bufferSize; //in bits!
		//flow spec	
		double peakRate; //in bit/sec
		//QoS requirements
		double pLoss; //max overflow probability
		//current number of admitted flows, number of rejections and flow managment
		int numFlows; int numRejects; int maxFlows; //max admissible number for SSUM algorithm
		double flowTimeout, checkPeriod;
		bool trace;
	} pp;
	
	//estimator settings
	struct estimatorParameter{
		estimatorType estType;
		acParameterSKDE *acpskde;
	} ep;
	
	//save init routine for default settings, default AC is SSUM
	inline void init(){
		dscp=0; adcType=SSUM;
		pp.polType=ACCEPT; //irrelevant for SSUM, default for all MBACs
		pp.capacity=1e6;/*1Mbit*/ pp.bufferSize=1.5e3; /*bit*/ pp.peakRate=8.16e4;/*(G.711+RTP/UDP/IP/ETH)*/ pp.pLoss=10e-5;
		pp.numFlows=0; 	pp.numRejects=0; pp.maxFlows=1; pp.flowTimeout=DEFAULT_FLOW_TIMEOUT; pp.checkPeriod=DEFAULT_CHECK_PERIOD; 
		pp.trace=false;
		ep.estType=NLEST; ep.acpskde=NULL;
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