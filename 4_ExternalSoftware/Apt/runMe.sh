#!/bin/bash 
#1)merged File path
#2) docker temp
#3) Final Docker name
#4) Final path
#5) path to hostPath txt file.
retry() {
    until "$@"
    do
            echo "Attempt failed! Trying again"
    done
}
if [ $# -ne 4 ] &&  [ $# -ne 5 ]
  then
    echo "You need to provide :"
        echo " The absolute path to the mergedDockers  "
    echo " a temporary docker name  "
    echo " DockerName to build the new docker container "
        echo " A temporary folder path "
    echo "Path to a file containing the hostPath, for Indocker user only "
   echo $#
    exit
fi

if [ $# -eq 4 ]
then
echo "hope you are on IOS or Linux"
pathSharedfoldDock=$4
pathSharedfoldHost=$4
fi

if [ $# -eq 5 ]
then
echo "WORKS ONLY IN DOCKER CONTAINER!!!!!!!!!!!!!!!!!!!!"
pathSharedfoldDock=$4
pathSharedfoldHost=$(cat $5)
if ! test -f "$5"; then
    echo "ConfigurationFile does not exists. You dont have access to the dockerFileGenerator power. Run again the script for DockerFileGeneratorGUI to recreate the file."
    echo "Docker container failed!! check log"
    exit 1
fi
echo $pathSharedfoldHost
fi

dockerTempName=$(echo "$2" | tr '[:upper:]' '[:lower:]')
dockerName=$(echo "$3" | tr '[:upper:]' '[:lower:]')


j="$( basename "$1" )"
echo ${j}
pathSharedfoldDocks=$pathSharedfoldDock/${j}_wExternalFile
if [ -d "$pathSharedfoldDocks" ]; then
    echo "Error: ${pathSharedfoldDocks} already exists. Use another name or delete the folder!!!!"
  exit 1
else
  echo "Installing files in ${pathSharedfoldDocks}..."

fi

mkdir -p $pathSharedfoldDock/${j}_wExternalFile


retry cp -r $1/* $pathSharedfoldDock/${j}_wExternalFile
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches

retry cp -r ./configurationFile.txt $pathSharedfoldDock/${j}_wExternalFile/
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
retry cp -r ./downloadSoft.sh $pathSharedfoldDock/${j}_wExternalFile/
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches


if ! docker build --platform linux/amd64 $pathSharedfoldDock/${j}_wExternalFile -t $dockerTempName; then
    exit 1
fi

echo "Step library Install. Might take some time. "
if ! docker run --platform linux/amd64 -tv $pathSharedfoldHost/${j}_wExternalFile/:/scratch $dockerTempName bash -c "cat /scratch/configurationFile.txt | /scratch/downloadSoft.sh"; then
echo "Wewweee uagliungell acca c'è qualcosa che non va sfaccimm!!!"
exit 1
fi
cat ./tail.txt >> $pathSharedfoldDock/${j}_wExternalFile/Dockerfile
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
echo 'DockerFile generation is done. Locate in DockerFolder and build your final docker.\n'

