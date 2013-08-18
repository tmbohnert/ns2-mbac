double acOQAEstimator::updateMeanIe(double ie, double t, int state){
	double ieHat=0.0;
	double tt=0.0;
	double check=0;

	// add the latest estimate and update the head pointer (ring buffer)
	if(state==1){
		ep.eModel.ieRec[ep.eModel.ieRecHead].ieb=ie;
		ep.eModel.ieRec[ep.eModel.ieRecHead].blen=t;
		ep.eModel.ieRec[ep.eModel.ieRecHead].state=state;
		ep.eModel.ieRecHead=(++ep.eModel.ieRecHead)%(ep.eModel.ieRecTotLen);
	}else{
		ep.eModel.ieRec[ep.eModel.ieRecHead].ieg=ie;
		ep.eModel.ieRec[ep.eModel.ieRecHead].glen=t;
		ep.eModel.ieRec[ep.eModel.ieRecHead].state=state;
		ep.eModel.ieRecHead=(++ep.eModel.ieRecHead)%(ep.eModel.ieRecTotLen);
	}	

	//compute the mean over the last T (wLen) sec
	for(int idx=ep.eModel.ieRecHead-1; tt<ep.eModel.wLen && check<=1; idx--){
		if(idx<0){
			idx=ep.eModel.ieRecTotLen-1; //negative idx, jump to the end of the vector
			check++; //maximum two loops
		}
		tt+=ep.eModel.ieRec[idx].t;
		ieHat+=(ep.eModel.ieRec[idx].t*ep.eModel.ieRec[idx].ie);
		//if(now>200)printf("STATE: %i, T: %lf, IE: %lf TT: %lf\n", ep.eModel.ieRec[codePt][idx].state, ep.eModel.ieRec[codePt][idx].t, ep.eModel.ieRec[codePt][idx].ie, tt);
	}
	ep.eModel.mieHat=ieHat/tt;
	//printf("STATE: %i, TT: %lf, IEHAT: %lf, MIEHAT: %lf\n", state, tt, ieHat, ep.eModel.mieHat[codePt]);
	return(ep.eModel.mieHat);
}