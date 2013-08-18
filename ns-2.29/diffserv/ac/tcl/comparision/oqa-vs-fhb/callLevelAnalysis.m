% Thomas Michael Bohnert
% Tampere, Sept. 2008
%
% ICC 2008 Paper
%
% Call level analysis
clear all;

k=1;
 for x=80:2:88
	disp(['=============================']);
% 	filename=['res-r' num2str(x) '.txt']
	filename=['r-analysis-fhb-rvalue-only.txt']
	d(k).rscore=load(filename);
	Rt=x; % QoS target

	% average, stddev, variance
	mu=mean(d(k).rscore)
 	sig=std(d(k).rscore)
	mi=min(d(k).rscore)
	mx=max(d(k).rscore)
	bad=numel(find(d(k).rscore<Rt))
	good=numel(d(k).rscore)-bad
	k=k+1;
end

col=jet(k-1);
figure(1);clf;hold on;
for i=1:k-1
	[y,x]=hist(d(i).rscore,100);
	plot(x,cumsum(y)/sum(y),'color',col(i,:));
end
xlabel(['Duration of bad period (R-score < R\prime)']);
ylabel('CDF');
legend('80','82','84','86','88');

	