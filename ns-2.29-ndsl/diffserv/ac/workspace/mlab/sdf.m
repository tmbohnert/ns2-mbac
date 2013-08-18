/********************************************************
SDF.PRC

bothom:mbac

Original version written by
Bruce E. Hansen
Department of Economics
University of Wisconsin
www.ssc.wisc.edu/~bhansen

This procedure implement smoothed distribution function estimation and bandwidth selection 
as described in "Bandwidth Selection for Nonparametric Distribution Estimation."

The SDF is implemented using a standard normal kernel.  
The bandwidth selection procedure can use any order plug-in. I recommend using a fourth-order plug-in.

For example, if your data vector is y, and the desired points of evaluation are x, then us
the commands:

h=Bandwidth(y,4);
f=SDF(x,y,h);

To plot the estimate, then use:
library pgraph;
xy(x,f);

*********************************************************/



/*****************************************
proc SDF

Format: f=SDF(x,y,h);

Inputs: x	qx1 points of evaluation
	y	nx1 data 
	h	1x1 bandwidth

Output:	f	qx1 SDF estimate

Note: The SDF is a smoothed version of the empirical distribution function, where
the smoothing is done with the normal cummulative distribution function (cdf). 
Here, we use the numerical approximation to the normal cdf given in equation (16.3.5) 
of "Computation of Special Functions" by Shanjie Zhang and Jianming Jin (1996), Wiley.

*****************************************/
proc SDF(x,y,h);
local f,u,c,t,er;
  if h .<=0;
    f=meanc(y .<= x');
  else;
    u=(x'-y)./h;
    let c = 1.330274429 1.821255978 1.781477937 .356563782 .31938153;
    t=1./(1+abs(u)*.2316419);
    er = 1 - ((((c[1].*t-c[2]).*t+c[3]).*t-c[4]).*t+c[5]).*t.*pdfn(u);
    f=(u.==0).*(.5)+(u.>0).*er+(u.<0).*(1-er);
    f=meanc(f);
  endif;
retp(f);
endp;
