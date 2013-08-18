% Generate Images

res=load('../allResultsInOne.txt');

% Hhat & Href
%genExpooImage(log10(res(1:4,1)), res(1:4, 6), res(5:8, 6));
%genPooImage(log10(res(1:4,1)), res(9:12, 6), res(13:16, 6));

%Hhat & POO & EXPOO
genHhatExpooPooImage(res(1:4,1), res(1:4, 6),  res(9:12, 6));

%genPrecHhatExpooPooImage(res(1:4,1)), log10(res(1:4, 2)),  log10(res(9:12, 2)));
%genPrecHhatExpooPooImage(res(1:4,1), res(1:4, 2),  res(9:12, 2));

