%/*****************************************
% Thomas Bohnert
% bothom:mbac
%
% Erweiterte, geschwindigkeits optimierter Kernschätzer fuer die "Roughness" von Ableitungen
% von Wahrscheinlichkeitsdichten
% Erweiterungen nach [Herrmann97], 3.4.5, FFT fuer Kernschätzer
%
% Fast Foursier Transform based Density Derivative Roughness Estimator
% Format: rm=fftddre(y, j)
% Input: y 1xn data 
% Output: rm=Roughness Schätzung der Ordnung m [Hansen04], (7) auch Jones
% and Sheather Schätzer.
%
% *****************************************/

function h = fftddre(y)
    n=numel(y);
    sigHat=std(y);
    
    % Default procedure follwing [Hansen04]
    m=1;  % the reference estimate corresponds to m=2, the estimate to m=1
    g=gamma(m+1.5);
    r=g/2/pi/(sigHat.^(m*2+3))	% Reference value for R_(J+1) %
    a=(gamma(m+.5)*(2^(m+.5))/pi/r/n)^(1/(2*m+3))	% bandwidth for roughness estimation %
     r=abs(mean(mean((normpdf(u).*hr), 2)))/(a^(1+2*m))	% Roughness estimation %
    
    % Speed optimized procedure follwing [Herrmann97]
    ymi=min(y);
    ymx=max(y);
    x=[ymi:(ymx-ymi)/(n-1):ymx];    %evaluation points
    ys=sort(y); 
    a=zeros(1, numel(x));     %weights
    j=2;    % range index (j) [Herrmann97]
    for i=1:numel(ys)   % sample index (i) [Herrmann97]
        while(ys(i)>x(j))
            j=j+1;
        end;
        omega=(ys(i)-x(j-1))/(x(j)-x(j-1));   % x(j)<=Y(i)<x(j+1)
        a(j)=a(j)+omega;
        a(j-1)=a(j-1)+1-omega;
   end;
    
    % calculate the estimate based on the standard procedure
    for i=1:n
        yy(i,:)=y'-y(i);
    end    
    u=yy./a;
    h0=1; h1=u;					% Hermite polynomial calculation %
    for i=1:1:2*j-1
        hr=u.*h1-i*h0;
        h0=h1;  h1=hr;
    end;
    s=sum(normpdf(u).*hr);
   
  
    
    
    
    
      