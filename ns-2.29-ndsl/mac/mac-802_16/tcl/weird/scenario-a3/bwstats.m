

function [bw] = bwstats(data, flen)

sz=size(data);
mxi=sz(1,1);
i=1
while(i<mxi)
    vol=0; z=0;
    while(data(i,1)==data(i+z,1) && (i+z) < mxi)
        vol=vol+data(i+z,2);
        z=z+1;
    end
    bw(i)=vol*8/flen
end    
        
    