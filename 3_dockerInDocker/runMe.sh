#!/bin/bash 
if [ $# -ne 3 ]
  then
    echo "You need to provide :"
        echo " The folderName of the mergedDockers  "
    echo " Folder name, for the new docker  "
    echo " DockerName to build the new docker container "
    exit
fi
mkdir ./$2


cp -r ./$1/* ./$2


cp ./$1/Dockerfile ./$2/Dockerfile
cat ./tail >> ./$2/Dockerfile
