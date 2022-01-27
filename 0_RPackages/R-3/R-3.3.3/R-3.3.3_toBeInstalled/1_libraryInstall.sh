#!/bin/bash 
nohup Rscript /scratch/libraryInstall.R > /scratch/nohup.out
Rscript /scratch/dockerFileGenerator.R
rm -r /scratch/packages
rm /scratch/nohup.out
rm /scratch/toDownload.txt
rm /scratch/toInstall.txt
