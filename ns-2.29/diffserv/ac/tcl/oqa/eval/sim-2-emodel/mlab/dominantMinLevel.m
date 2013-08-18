% Thomas Michael Bohnert
% Tampere, Sept. 2008
%
% ICC 2008 Paper
%
% Calculate the longest duration of a quality level below a certain minimum

clear all;

k=1;
% for x=80:2:88
for x=85
	disp(['=============================']);
	filename=['acTrace-0-r' num2str(x) '.tr']
	d=load(filename);
	Rt=x; % QoS target

	% dissmiss the first 500s
	idx=find(d(:,2)>500);
	% extract the R-Scores
	vr=d(idx(1):end, 5);
	%arrival times
	at=d(idx,2); 
	%values below Rtarget
	x=vr<Rt;
	
	G2B=find(x(1:end-1)<x(2:end));
	B2G=find(x(1:end-1)>x(2:end));
	if x(1)==1
		if x(end)==0
			BadTimes=at(B2G(2:end))-at(G2B(1:end));
		else
			BadTimes=at(B2G(2:end))-at(G2B(1:end-1));
		end
	else
		if x(end)==0
			BadTimes=at(B2G(1:end))-at(G2B(1:end));
		else
			BadTimes=at(B2G(1:end))-at(G2B(1:end-1));
		end        
	end
	z(k).BadTimes=BadTimes;k=k+1;
	
	% average, stddev, variance
	mu=mean(vr)
 	sig=std(vr)
	mi=min(vr)
	mx=max(vr)
	tmx=max(BadTimes)
end

col=jet(k-1);
figure(1);clf;hold on;
for i=1:k-1
	[y,x]=hist(z(i).BadTimes,100);
	plot(x,cumsum(y)/sum(y),'color',col(i,:));
end
xlabel(['Duration of bad period (R-score < R\prime)']);
ylabel('CDF');
legend('80','82','84','86','88');

	