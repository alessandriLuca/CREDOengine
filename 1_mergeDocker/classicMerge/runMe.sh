#!/bin/bash 
retry() {
    until "$@"
    do
            echo "Attempt failed! Trying again"
    done
}
if [ $# -eq 0 ]
  then
    echo "You need to provide a name for the merged Docker"
    exit
fi
mkdir -p $4/$1


retry cp -r $2/* $4/$1
sync
retry cp -r $3/* $4/$1
sync
echo "FROM library/ubuntu:20.04 as UBUNTU_BASE
MAINTAINER alessandri.luca1991@gmail.com
ARG DEBIAN_FRONTEND=noninteractive" > $4/$1/Dockerfile

tail -n +4 $2/Dockerfile >> $4/$1/Dockerfile
sync
tail -n +4 $3/Dockerfile >> $4/$1/Dockerfile
sync
