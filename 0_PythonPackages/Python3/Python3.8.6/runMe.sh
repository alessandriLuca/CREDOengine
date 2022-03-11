#!/bin/bash
retry() {
    until "$@"
    do
            echo "Attempt failed! Trying again"
    done
}
if [ $# -eq 0 ]
  then
    echo "You need to provide a docker temporary tag a final name and a temporary folder path. For windows add also the host path to a file containing the sharedFolder path. for example ./buildMe.sh dockertest resultDockerfile /home/user/temporaryFolder"
    exit
fi

if [ $# -eq 1 ]
  then
    echo "You need to provide a docker temporary tag a final name and a temporary folder path. For windows add also the host path to the sharedFolder. for example ./buildMe.sh dockertest resultDockerfile /home/user/temporaryFolder"
    exit
fi

if [ $# -eq 2 ]
  then
    echo "You need to provide a docker temporary tag a final name and a temporary folder path. For windows add also the host path to the sharedFolder. for example ./buildMe.sh dockertest resultDockerfile /home/user/temporaryFolder"
    exit
fi

if [ $# -eq 3 ]
then
echo "hope you are on IOS or Linux"
pathSharedfoldDock=$3/$2
pathSharedfoldHost=$3/$2
mkdir -p $pathSharedfoldDock
echo "hey"
fi

if [ $# -eq 4 ]
then
echo "WORKS ONLY IN DOCKER CONTAINER!!!!!!!!!!!!!!!!!!!!"
pathSharedfoldDock=$3/$2
somedirpath=$(cat $4)
pathSharedfoldHost="$somedirpath"/"$( basename "$pathSharedfoldDock" )"
if ! test -f "$4"; then
    echo "ConfigurationFile does not exists. You dont have access to the dockerFileGenerator power. Run again the script for DockerFileGeneratorGUI to recreate the file."
    echo "Docker container failed!! check log"
    exit 1
fi
echo $pathSharedfoldHost

fi
if [ -d "$pathSharedfoldDock" ]; then
    echo "Error: ${pathSharedfoldDock} already exists. Use another name or delete the folder!!!!"
  exit 1
else
  echo "Installing files in ${pathSharedfoldDock}..."

fi
mkdir -p $pathSharedfoldDock
mv Dockerfile_1 Dockerfile
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
dockerTempName=$(echo "$1" | tr '[:upper:]' '[:lower:]')
echo $dockerTempName
if ! docker build --platform linux/amd64 . -t $dockerTempName; then
    echo "Docker container failed!! check log"
    exit 1
fi
retry cp -r . $pathSharedfoldDock
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
rm $pathSharedfoldDock/Dockerfile*
retry cp configurationFile.sh $pathSharedfoldDock/Python3.8.6_toBeInstalled/configurationFile.sh
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
echo "Step : python library install. This might take some time."
if ! docker run --platform linux/amd64 -tv $pathSharedfoldHost/Python3.8.6_toBeInstalled:/scratch $dockerTempName /scratch/1_libraryInstall.sh; then
    echo "Docker container failed!! check log"
    exit 1
fi
mv Dockerfile Dockerfile_1
mv Dockerfile_2 Dockerfile
mv Dockerfile Dockerfile_2

retry cp Dockerfile_2 $pathSharedfoldDock/Dockerfile
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
if ! docker build --platform linux/amd64 $pathSharedfoldDock/ -t $dockerTempName; then
    echo "Docker container failed!! check log"
    exit 1
fi
echo 'DockerFile generation is done. Locate in DockerFolder and build your final docker.\n You can remove the temporary docker with docker rmi '$dockerTempName

