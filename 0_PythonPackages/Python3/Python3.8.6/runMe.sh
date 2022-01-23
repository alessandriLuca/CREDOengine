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


#temp finalName tempFolder-> /sharedFolder pathToTempFolderOnHost-> Leggi da file

#docker rmi -f $1
mv Dockerfile_1 Dockerfile
docker build . -t $1
#cp -R ./Python3.8.6_toBeInstalled $pathSharedfoldDock
cp -r . $pathSharedfoldDock
rm $pathSharedfoldDock/Dockerfile*
cp configurationFile.sh $pathSharedfoldDock/Python3.8.6_toBeInstalled/configurationFile.sh
docker run -tv $pathSharedfoldHost/Python3.8.6_toBeInstalled:/scratch $1 /scratch/1_libraryInstall.sh # DEVE ESSERE IL PATH DI HOST, DEVE ESSERE LA SHARED FOLDER
mv Dockerfile Dockerfile_1
mv Dockerfile_2 Dockerfile
mv Dockerfile Dockerfile_2

cp Dockerfile_2 $pathSharedfoldDock/Dockerfile
#cp Python-3.8.6.tgz ./$pathSharedfoldDock/
#mkdir ./$2/Python3.8.6_toBeInstalled
#cp $pathSharedfoldDock/Python3.8.6_toBeInstalled/*.7z* ./$2/Python3.8.6_toBeInstalled/
#cp ./pipdeptree-2.1.0-py3-none-any.whl ./$2/
#cp -r ./p7zip_16.02 ./$2/
#rm -r $pathSharedfoldDock/Python3.8.6_toBeInstalled/packages
#rm $pathSharedfoldDock/Python3.8.6_toBeInstalled/1_libraryInstall.sh
#rm $pathSharedfoldDock/Python3.8.6_toBeInstalled/configurationFile.sh
#rm $pathSharedfoldDock/Python3.8.6_toBeInstalled/*.txt
#rm $pathSharedfoldDock/Python3.8.6_toBeInstalled/*.log
#cp -r $pathSharedfoldDock/Python3.8.6_toBeInstalled/ ./$2/
echo 'DockerFile generation is done. Locate in DockerFolder and build your final docker.\n You can remove the temporary docker with docker rmi '$1
#rm -r $pathSharedfoldDock
#docker rmi -f $1
