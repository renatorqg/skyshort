#!/bin/bash
echo generating the gnuplot graphics
cp ./gnuplot-scripts/B_Y-all.p ./datapoints/B_Y/
cd ./datapoints/B_Y
gnuplot B_Y-all.p
cd ../..
cp ./gnuplot-scripts/gabor0-all.p ./datapoints/gabor0/
cd ./datapoints/gabor0
gnuplot gabor0-all.p
cd ../..
cp ./gnuplot-scripts/gabor45-all.p ./datapoints/gabor45/
cd ./datapoints/gabor45
gnuplot gabor45-all.p
cd ../..
cp ./gnuplot-scripts/gabor90-all.p ./datapoints/gabor90/
cd ./datapoints/gabor90
gnuplot gabor90-all.p
cd ../..
cp ./gnuplot-scripts/gabor135-all.p ./datapoints/gabor135/
cd ./datapoints/gabor135
gnuplot gabor135-all.p
cd ../..
cp ./gnuplot-scripts/gray-all.p ./datapoints/gray/
cd ./datapoints/gray
gnuplot gray-all.p
cd ../..
cp ./gnuplot-scripts/motion-B_Y-all.p ./datapoints/motion-B_Y/
cd ./datapoints/motion-B_Y
gnuplot motion-B_Y-all.p
cd ../..
cp ./gnuplot-scripts/motion-gabor0-all.p ./datapoints/motion-gabor0/
cd ./datapoints/motion-gabor0
gnuplot motion-gabor0-all.p
cd ../..
cp ./gnuplot-scripts/motion-gabor45-all.p ./datapoints/motion-gabor45/
cd ./datapoints/motion-gabor45
gnuplot motion-gabor45-all.p
cd ../..
cp ./gnuplot-scripts/motion-gabor90-all.p ./datapoints/motion-gabor90/
cd ./datapoints/motion-gabor90
gnuplot motion-gabor90-all.p
cd ../..
cp ./gnuplot-scripts/motion-gabor135-all.p ./datapoints/motion-gabor135/
cd ./datapoints/motion-gabor135
gnuplot motion-gabor135-all.p
cd ../..
cp ./gnuplot-scripts/motion-gray-all.p ./datapoints/motion-gray/
cd ./datapoints/motion-gray
gnuplot motion-gray-all.p
cd ../..
cp ./gnuplot-scripts/motion-R_G-all.p ./datapoints/motion-R_G/
cd ./datapoints/motion-R_G
gnuplot motion-R_G-all.p
cd ../..
cp ./gnuplot-scripts/motion-RGB-all.p ./datapoints/motion-RGB/
cd ./datapoints/motion-RGB
gnuplot motion-RGB-all.p
cd ../..
cp ./gnuplot-scripts/motion-sr-all.p ./datapoints/motion-sr/
cd ./datapoints/motion-sr
gnuplot motion-sr-all.p
cd ../..
cp ./gnuplot-scripts/R_G-all.p ./datapoints/R_G/
cd ./datapoints/R_G
gnuplot R_G-all.p
cd ../..
cp ./gnuplot-scripts/sr-all.p ./datapoints/sr/
cd ./datapoints/sr
gnuplot sr-all.p
cd ../..

