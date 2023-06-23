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

pathSharedfoldDocks=$2/${j}_wExternalFile
if [ -d "$pathSharedfoldDocks" ]; then
    echo "Error: ${pathSharedfoldDocks} already exists. Use another name or delete the folder!!!!"
  exit 1
else
  echo "Installing files in ${pathSharedfoldDocks}..."

fi
mkdir $2/${j}_wExternalFile


retry cp -r $1/* $2/${j}_wExternalFile
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches

retry cp downloadSoft.sh $2/${j}_wExternalFile

sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches

retry cp ./softwareList.txt $2/${j}_wExternalFile

sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches

if ! docker build --platform linux/amd64 $2/${j}_wExternalFile/ -t youwillneverusethiname; then
    echo "Docker container failed!! check log"
    exit 1
fi

if ! docker run -tv $2/${j}_wExternalFile:/scratch youwillneverusethiname bash -c "cat /scratch/softwareList.txt | /scratch/downloadSoft.sh"; then
echo "Wewweee uagliungell acca c'è qualcosa che non va sfaccimm!!!"
exit 1
fi
cat tail.txt >> $2/${j}_wExternalFile/Dockerfile

echo 'DockerFile generation is done. Locate in DockerFolder and build your final docker.\n'
