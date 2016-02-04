# gs - Gnuplot Script file for plotting all sr 
# to run type gnuplot "namefile.p"

set term png size 780, 400
set output "graph-SR-0.png"
set key out right top
plot "./0-sec0" with lp, "./0-sec1" with lp, "./0-sec2" with lp, "./0-sec3" with lp, "./0-sec4" with lp, "./0-sec5" with lp, "./0-sec6" with lp, "./0-sec7" with lp, "./0-sec8" with lp

set term png size 780, 400
set output "graph-SR-2.png"
set key out right top
plot "./2-sec0" with lp, "./2-sec1" with lp, "./2-sec2" with lp, "./2-sec3" with lp, "./2-sec4" with lp, "./2-sec5" with lp, "./2-sec6" with lp, "./2-sec7" with lp, "./2-sec8" with lp

set term png size 780, 400
set output "graph-SR-3.png"
set key out right top
plot "./3-sec0" with lp, "./3-sec1" with lp, "./3-sec2" with lp, "./3-sec3" with lp, "./3-sec4" with lp, "./3-sec5" with lp, "./3-sec6" with lp, "./3-sec7" with lp, "./3-sec8" with lp

set term png size 780, 400
set output "graph-SR-4.png"
set key out right top
plot "./4-sec0" with lp, "./4-sec1" with lp, "./4-sec2" with lp, "./4-sec3" with lp, "./4-sec4" with lp, "./4-sec5" with lp, "./4-sec6" with lp, "./4-sec7" with lp, "./4-sec8" with lp

set term png size 780, 400
set output "graph-SR-5.png"
set key out right top
plot "./5-sec0" with lp, "./5-sec1" with lp, "./5-sec2" with lp, "./5-sec3" with lp, "./5-sec4" with lp, "./5-sec5" with lp, "./5-sec6" with lp, "./5-sec7" with lp, "./5-sec8" with lp


