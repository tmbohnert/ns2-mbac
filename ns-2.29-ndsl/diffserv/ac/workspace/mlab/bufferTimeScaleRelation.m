%
% Pufferlänge und Zeitbereich, Verhältnis
%
% Thomas Bohnert, 16.10.2006

tdm=1.5*10^-3;  % maximal delay, i.e. buffer length
c=10*10^6;

omega=c*tdm;    %buffer length for max delay

idx=1;
a=[0.02:0.02:0.1];
r=0;
for t=a
    w(idx)=c*t;
    r(idx)=omega/w(idx);
    idx=idx+1;
end;
r
plot(a, r, '--bd');