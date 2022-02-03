#!/bin/bash 
if ! nohup Rscript /scratch/libraryInstall.R > /scratch/nohup.out; then
    exit 1
fi
if ! Rscript /scratch/dockerFileGenerator.R; then
    exit 1
fi
rm -r /scratch/packages
rm /scratch/nohup.out
rm /scratch/toDownload.txt
rm /scratch/toInstall.txt

