#!/bin/bash 
nohup Rscript /scratch/libraryInstall.R > /scratch/nohup.out
Rscript /scratch/dockerFileGenerator.R
