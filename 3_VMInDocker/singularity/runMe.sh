#!/bin/bash 
retry() {
    until "$@"
    do
            echo "Attempt failed! Trying again"
    done
}
if [ $# -ne 2 ]
  then
    echo "You need to provide :"
        echo " The folderName of the mergedDockers  "
                echo " The result Folder"
    exit
fi
j="$( basename "$1" )"
pathSharedfoldDocks=$2/${j}_wSingularity
if [ -d "$pathSharedfoldDocks" ]; then
    echo "Error: ${pathSharedfoldDocks} already exists. Use another name or delete the folder!!!!"
  exit 1
else
  echo "Installing files in ${pathSharedfoldDocks}..."

fi
mkdir $2/${j}_wSingularity


retry cp -r $1/* $2/${j}_wSingularity
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
retry cp -r ./* $2/${j}_wSingularity
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
cat ./tail >> $2/${j}_wSingularity/Dockerfile

if ! docker build --platform linux/amd64 $2/${j}_wSingularity/ -t youwillneverusethiname; then
    echo "Docker container failed!! check log"
    exit 1
fi
echo 'DockerFile generation is done. Locate in DockerFolder and build your final docker.'
