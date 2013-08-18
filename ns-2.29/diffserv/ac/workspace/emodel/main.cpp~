//
// C++ Implementation: emodel
//
// Description: Objective/Instrumental Quality Assessment for a VoIP call
//
//
// Author: Thomas Michael Bohnert <tmb@nginet.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include "exception.h"
#include "emodel.h" //MAXDATALEN

using namespace std;


int main(int argc, char *argv[]){
	double frlen=atof(argv[1]);
 	double bufferSize=atof(argv[2]);
	double capacity=pow(10.0, atof(argv[3]));

	EModel em(MAXDATALEN, frlen);
	char data[MAXDATALEN];
	double rScoreHat[MAXDATALEN];
	int idx=0;

	double pkts;
	double rcvd=0.0;
	double lost=0.0;

	try {
		ifstream fin(argv[4]);
		if (!fin) throw Exception("Couldn't find input file");

		while(!fin.eof()){
			printf("\n\n++++++++++++++++++++ New flow detected ++++++++++++++++++++\n");
			fin.getline(data, MAXDATALEN);
			if(fin.gcount()==MAXDATALEN)
				printf("Buffer too small, proceed with next flow\n");
			for(int i=0; i<fin.gcount(); i++){
				if(data[i]=='+'){
					pkts++;
					rcvd++;
					//if(!remainder(rcvd, 500)) printf("More 500 packets reveived\n");
					em.meterUpdate(1); //received packet
				}	
				if(data[i]=='-'){
					pkts++;
					lost++;
					//if(!remainder(rcvd, 500)) printf("More 500 packets lost\n");
					em.meterUpdate(0); //lost packet
				}	
			}
			rScoreHat[idx++]=(94-(4+1*(bufferSize/capacity))-em.estimate());
			printf("#pkts=%lf, #received=%lf, #losses=%lf, lratio=%lf, RScoreHat=%lf\n------------------------------------------\n",pkts, rcvd, lost, lost/pkts, rScoreHat[idx-1]);
			if(idx>MAXDATALEN){
				printf("Maximum number of results reached. Terminate processing\n");
				break;
			}else{
				em.reset(MAXDATALEN, frlen);
				pkts=rcvd=lost=0.0;
			}	
		}
	}catch (Exception e) {
		cout << e.getMessage() << '\n';
		return -1;
	}

	for(int i=0; i<idx; i++)
		printf("RScoreHat=%lf\n", rScoreHat[i]);
	return 0;
}
