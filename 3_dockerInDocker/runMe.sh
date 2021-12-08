#!/bin/bash 
if [ $# -ne 2 ]
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

echo "docker build . -t "$2 > ./$1_wDocker/script.sh
echo "docker run -itv $HOME/JupyterFolder:/sharedFolder -v /var/run/docker.sock:/var/run/docker.sock --privileged=true --volume=$HOME/.Xauthority:/root/.Xauthority:rw  -p  8888:8888 "$2 >> ./$1_wDocker/script.sh

