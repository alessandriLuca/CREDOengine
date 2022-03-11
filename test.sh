#!/bin/bash
allThreads=(./0_PythonPackages/Python2/Python2.7.18/ ./0_PythonPackages/Python3/Python3.8.6/ ./0_RPackages/R-2/R-2.11.1/ ./0_RPackages/R-2/R-2.12.2/ ./0_RPackages/R-2/R-2.13.2/ ./0_RPackages/R-2/R-2.14.2/ ./0_RPackages/R-2/R-2.15.3/ ./0_RPackages/R-3/R-3.0.3/ ./0_RPackages/R-3/R-3.1.3/ ./0_RPackages/R-3/R-3.2.5/ ./0_RPackages/R-3/R-3.3.3/ ./0_RPackages/R-3/R-3.4.4/ ./0_RPackages/R-3/R-3.5.3/ ./0_RPackages/R-3/R-3.6.3/ ./0_RPackages/R-4/R-4.0.5/ ./0_RPackages/R-4/R-4.1.1/) 

home=$(pwd)
for t in ${allThreads[@]}; do
  cd $t && ./example.sh
  cd $home
done
