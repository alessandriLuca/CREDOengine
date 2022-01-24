#!/bin/bash
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
echo $pathSharedfoldHost
mkdir -p $pathSharedfoldDock

fi

mv Dockerfile_1 Dockerfile
sync
docker build . -t $1

cp -r . $pathSharedfoldDock
sync
rm $pathSharedfoldDock/Dockerfile*
sync
cp configurationFile.R $pathSharedfoldDock/jennifer_toBeInstalled/libraryInstall.R
sync
docker run -tv $pathSharedfoldHost/jennifer_toBeInstalled:/scratch $1 /scratch/1_libraryInstall.sh
mv Dockerfile Dockerfile_1
sync
mv Dockerfile_2 Dockerfile
sync
mv Dockerfile Dockerfile_2
sync
cp Dockerfile_2 $pathSharedfoldDock/Dockerfile
sync
echo 'DockerFile generation is done. Locate in DockerFolder and build your final docker.\n You can remove the temporary docker with docker rmi '$1

