/********************************************************
SDF.PRC

bothom:mbac

written by
Bruce E. Hansen
Department of Economics
University of Wisconsin
www.ssc.wisc.edu/~bhansen

These two procedures implement smoothed distribution function estimation and bandwidth selection 
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


/*****************************************
proc Bandwidth

Format: h=Bandwidth(y,j)

Input: 	y	nx1 data 
	j	Plug-in Order

Output:	h	Bandwidth

This procedure computes a plug-in bandwidth for smooth cdf estimation.

If j=0, h is the Gaussian reference bandwidth.
If j>0, h is the j-step plug-in bandwidth.
Simulations suggest that j=4 is a good choice for practice.

*****************************************/

proc Bandwidth(y,j);
local n,yy,r,m,a,u,h0,h1,hr,h,i; 
  n=rows(y);
  yy=y-y';
  r=gamma(j+1.5)/2/pi/(stdc(y).^(2*j+3));	@ Reference value for R_(J+1) @
  for m (j,1,-1);				@ Sequential Estimation of R_m @
    a=(gamma(m+.5)*(2^(m+.5))/pi/r/n)^(1/(2*m+3));	@ bandwidth for roughness estimation @
    u=yy/a;
    h0=1;h1=u;					@ Hermite polynomial calculation @
    for i (1,2*m-1,1);
      hr=u.*h1-i*h0;
      h0=h1; h1=hr;
    endfor;
    r=abs(meanc(meanc(pdfn(u).*hr)))/(a^(1+2*m));	@ Roughness estimation @
  endfor;
  h=1/((sqrt(pi)*r*n)^(1/3));			@ Plug-in Bandwidth @
retp(h);
endp;
