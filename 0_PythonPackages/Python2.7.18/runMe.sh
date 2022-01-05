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
pathSharedfoldDock=$3
pathSharedfoldHost=$3
mkdir $pathSharedfoldDock
echo "hey"
fi

if [ $# -eq 4 ]
then
echo "WORKS ONLY IN DOCKER CONTAINER!!!!!!!!!!!!!!!!!!!!"
pathSharedfoldDock=$3
somedirpath=$(cat $4)
pathSharedfoldHost="$somedirpath"/"$( basename "$pathSharedfoldDock" )"
echo $pathSharedfoldHost
mkdir $pathSharedfoldDock

fi


#temp finalName tempFolder-> /sharedFolder pathToTempFolderOnHost-> Leggi da file

#docker rmi -f $1
mv Dockerfile_1 Dockerfile
docker build . -t $1
cp -R ./Python2.7.18_toBeInstalled $pathSharedfoldDock
cp configurationFile.sh $pathSharedfoldDock/Python2.7.18_toBeInstalled/configurationFile.sh
docker run -v $pathSharedfoldHost/Python2.7.18_toBeInstalled:/scratch $1 /scratch/1_libraryInstall.sh # DEVE ESSERE IL PATH DI HOST, DEVE ESSERE LA SHARED FOLDER
mv Dockerfile Dockerfile_1
mv Dockerfile_2 Dockerfile
mv Dockerfile Dockerfile_2
mkdir $2
cp Dockerfile_2 ./$2/Dockerfile
cp Python-2.7.18.tgz ./$2/
mkdir ./$2/Python2.7.18_toBeInstalled
cp $pathSharedfoldDock/Python2.7.18_toBeInstalled/*.7z* ./$2/Python2.7.18_toBeInstalled/
cp ./pipdeptree-2.1.0-py2-none-any.whl ./$2/
cp -r ./p7zip_16.02 ./$2/
rm $pathSharedfoldDock/Python2.7.18_toBeInstalled/1_libraryInstall.sh
rm $pathSharedfoldDock/Python2.7.18_toBeInstalled/configurationFile.sh
rm $pathSharedfoldDock/Python2.7.18_toBeInstalled/*.txt
rm $pathSharedfoldDock/Python2.7.18_toBeInstalled/*.log
cp -r $pathSharedfoldDock/Python2.7.18_toBeInstalled/ ./$2/
echo 'DockerFile generation is done. Locate in DockerFolder and build your final docker.\n You can remove the temporary docker with docker rmi '$1
rm -r $pathSharedfoldDock
#docker rmi -f $1
