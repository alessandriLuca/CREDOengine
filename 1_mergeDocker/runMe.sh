#!/bin/bash 
if [ $# -eq 0 ]
  then
    echo "You need to provide a name for the merged Docker"
    exit
fi
VARIABLE=$(ls -d */)
mkdir ./$1

for V in $VARIABLE
do
cp -r ./$V/* ./$1
done

echo "FROM library/ubuntu as UBUNTU_BASE
MAINTAINER alessandri.luca1991@gmail.com
ARG DEBIAN_FRONTEND=noninteractive" > ./$1/Dockerfile
for V in $VARIABLE
do
tail -n +4 ./$V/Dockerfile >> ./$1/Dockerfile
done

