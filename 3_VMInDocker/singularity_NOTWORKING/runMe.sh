#!/bin/bash 
if [ $# -ne 1 ]
  then
    echo "You need to provide :"
        echo " The folderName of the mergedDockers  "
    echo " Folder name, for the new docker  "
    echo " DockerName to build the new docker container "
    exit
fi
mkdir ./$1_wDocker


cp -r ./$1/* ./$1_wDocker


cp ./$1/Dockerfile ./$1_wDocker/Dockerfile
cat ./tail >> ./$1_wDocker/Dockerfile

