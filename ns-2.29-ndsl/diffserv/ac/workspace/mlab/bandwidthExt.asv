%/*****************************************
% Thomas Bohnert
% bothom:mbac
%
% Erweiterte, geschwindigkeits optimierte function bandwidth() aus [Hansen04] in Matlab implementiert 
% Erweitrungen auf nach [Herrmann97], 3.4.5, FFT fuer Kernschätzer
%
% Format: h=Bandwidth(y,j)
% Input: y nx1 data 
% Plug-in Order: j
% Output:	h	Bandwidth
%
% This procedure computes a plug-in bandwidth for smooth cdf estimation.
% 
% If j=0, h is the Gaussian reference bandwidth.
% If j>0, h is the j-step plug-in bandwidth.
% Simulations suggest that j=4 is a good choice for practice.
% 
% *****************************************/

function h = bandwidth(y,j)
    n=numel(y);
    for i=1:n
        yy(i,:)=y'-y(i);
    end    
    sigHat=std(y);
    g=gamma(j+1.5);
    r=g/2/pi/(sigHat.^(2*j+3))	% Reference value for R_(J+1) %
    for m=j:-1:1				% Sequential Estimation of R_m %
        a=(gamma(m+.5)*(2^(m+.5))/pi/r/n)^(1/(2*m+3))	% bandwidth for roughness estimation %
        u=yy./a;
        h0=1;h1=u;					% Hermite polynomial calculation %
        for i=1:1:2*m-1
            hr=u.*h1-i*h0;
            h0=h1; h1=hr;
        end;
        s=sum(normpdf(u).*hr);
        r=abs(mean(mean((normpdf(u).*hr), 2)))/(a^(1+2*m))	% Roughness estimation %
    end;
    h=1/((sqrt(pi)*r*n)^(1/3));			% Plug-in Bandwidth %
    
    
    
    
    
    
    
    
    
    