% Generate Images

res=load('../allResultsInOne.txt');
genExpooImage(log10(res(1:4,1)), res(1:4, 6), res(5:8, 6));
genPooImage(log10(res(1:4,1)), res(9:12, 6), res(13:16, 6));