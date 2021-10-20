#!/bin/bash 
if [ $# -eq 0 ]
  then
    echo "You need to provide a docker tag and a $2 name, for example ./buildMe.sh dockertest tempFolder"
    exit
fi

if [ $# -eq 1 ]
  then
    echo "You need to provide a docker tag and a $2 name, for example ./buildMe.sh dockertest tempFolder"
    exit
fi
sudo docker rmi -f $1
mv Dockerfile_1 Dockerfile
sudo docker build . -t $1
cp configurationFile.sh ./Python3.8.6_toBeInstalled/
sudo docker run -itv $(pwd)/Python3.8.6_toBeInstalled:/scratch $1 /scratch/1_libraryInstall.sh
mv Dockerfile Dockerfile_1
mv Dockerfile_2 Dockerfile
sudo docker build . -t $1
mv Dockerfile Dockerfile_2
mkdir $2 
cp Dockerfile_2 ./$2/Dockerfile
cp Python-3.8.6.tgz ./$2/
mkdir ./$2/Python3.8.6_toBeInstalled
cp ./Python3.8.6_toBeInstalled/*.7z* ./$2/Python3.8.6_toBeInstalled/
cp ./pipdeptree-2.1.0-py3-none-any.whl ./$2/
cp -r ./p7zip_16.02 ./$2/
cp ./Python3.8.6_toBeInstalled/listForDockerfile.sh ./$2/Python3.8.6_toBeInstalled/listForDockerfile.sh
rm ./Python3.8.6_toBeInstalled/*.txt
rm ./Python3.8.6_toBeInstalled/*.log
rm ./Python3.8.6_toBeInstalled/*.7z*
rm -r ./Python3.8.6_toBeInstalled/packages
rm ./Python3.8.6_toBeInstalled/listForDockerfile.sh
rm ./Python3.8.6_toBeInstalled/configurationFile.sh
echo 'DockerFile generation is done. Locate in DockerFolder and build your final docker.'
sudo docker rmi -f $1
