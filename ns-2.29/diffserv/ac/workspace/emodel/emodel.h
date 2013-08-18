//
// C++ Interface: emodel
//
// Description: 
//
//
// Author: Thomas Michael Bohnert <tmb@nginet.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#define MAXDATALEN 100000
		
struct eModelParam {
	double frlen; 	//voip frame length in sec.
	bool state;	//0 = gap, 1 = burst
	long pkts; //total received packets
	int loss; 
	double pktRefSt;	//number of packet triggering a state transition
	double pktRefLe;	//number of last lost packet (loss event)
	int ieRecHead; int ieRecTotLen;
	double ieBHat; double bLenHat; //mean burst Ie and burst length
	double ieGHat; double gLenHat; //mean gap Ie and gap length
	double mieHat; //current mean Ie
	double rScoreHat; //estimated rScore

	struct ieRecord{
		double ie;	//Ie without gap/loss association for the model presented in [BohMon06, CCNC06]
		double t;		//time
		int state; //state of the four-state loss model
		double ieg; //next four variables are to calculate average integral quality, see [Clark01, Raake06]
		double ieb;
		double glen;
		double blen;
	} *ieRec;
	
	void inline initDefaults(int len){
		frlen=0.02; //20ms voip frames per packet		
		state=0; //loss gap
		pkts=0;
		loss=0;
		pktRefSt=0;
		pktRefLe=0;
		rScoreHat=100.0; //default estimated/measured quality
		initIeRec(len);
	}

	void inline reset(int len){
 		delete [] ieRec;
 		initDefaults(len);
	}
	
	void inline initIeRec(int len){
			if(len==0 || len > MAXDATALEN)
				len=MAXDATALEN;
			else
				ieRecTotLen=len;
			ieRec=new ieRecord[ieRecTotLen];
			ieRecHead=0;
			for(int i=0; i<ieRecTotLen-1; i++){
				ieRec[i].ie=0.0;
				ieRec[i].t=0.0;
				ieRec[i].ieg=0.0;
				ieRec[i].ieb=0.0;
				ieRec[i].glen=0.0;
				ieRec[i].blen=0.0;
			}
	};
};

class EModel {
private:
	eModelParam eModel;
public:
	EModel(int dataLen, double frlen);
	void reset(int dataLen, double frlen);
	void meterUpdate(double bytes);
	void updateModel();
	double lossImpairFactor(double lossRatio, int model);
	double updateIeRecord(double ie, double t, int state);
	double calcMeanIe();
	double estimate();
};
