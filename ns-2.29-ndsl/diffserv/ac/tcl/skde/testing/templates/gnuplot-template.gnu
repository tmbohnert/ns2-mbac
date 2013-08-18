#reset
#clear
set terminal png
#set key box
set key top right
set title "wlen 25"
set xlabel "Time"
set ylabel "Flow"
set xrange [500:4600]
#set logscale x
#set xtics (0.0001, 0.001, 0.01, 0,1)
#plot "acTrace-0_EST.tr" u ($2):(1-$3) t"Fxhref" w lines 1
plot "acTrace-0_FLO.tr" u ($2):($3) t"FLow" w lines 2
set output "output.png"
#replot
