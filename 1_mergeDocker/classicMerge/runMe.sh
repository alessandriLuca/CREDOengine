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
pathSharedfoldDock=$4/$1
if [ -d "$pathSharedfoldDock" ]; then
    echo "Error: ${pathSharedfoldDock} already exists. Use another name or delete the folder!!!!"
  exit 1
else
  echo "Installing files in ${pathSharedfoldDock}..."

fi
mkdir -p $4/$1


retry cp -r $2/* $4/$1
sync
echo 3 > /proc/sys/vm/drop_caches
retry cp -r $3/* $4/$1
sync
echo 3 > /proc/sys/vm/drop_caches
echo "FROM library/ubuntu:20.04 as UBUNTU_BASE
MAINTAINER alessandri.luca1991@gmail.com
ARG DEBIAN_FRONTEND=noninteractive" > $4/$1/Dockerfile

tail -n +4 $2/Dockerfile >> $4/$1/Dockerfile
sync
echo 3 > /proc/sys/vm/drop_caches
tail -n +4 $3/Dockerfile >> $4/$1/Dockerfile
sync
echo 3 > /proc/sys/vm/drop_caches
if ! docker build $4/$1/ -t youwillneverusethisname; then
    echo "Docker container failed!! check log"
    exit 1
fi
echo 'DockerFile generation is done.'
