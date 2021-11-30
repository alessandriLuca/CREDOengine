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

 docker rmi -f $1
mv Dockerfile_1 Dockerfile
 docker build . -t $1
cp configurationFile.sh ./Python2.7.18_toBeInstalled/configurationFile.sh
 docker run -itv $(pwd)/Python2.7.18_toBeInstalled:/scratch $1 /scratch/1_libraryInstall.sh
mv Dockerfile Dockerfile_1
mv Dockerfile_2 Dockerfile
 docker build . -t $1
mv Dockerfile Dockerfile_2
mkdir $2 
cp Dockerfile_2 ./$2/Dockerfile
cp Python-2.7.18.tgz ./$2/
mkdir ./$2/Python2.7.18_toBeInstalled
cp ./Python2.7.18_toBeInstalled/*.7z* ./$2/Python2.7.18_toBeInstalled/
cp ./pipdeptree-2.1.0-py2-none-any.whl ./$2/
cp -r ./p7zip_16.02 ./$2/
cp ./Python2.7.18_toBeInstalled/listForDockerfile.sh ./$2/Python2.7.18_toBeInstalled/listForDockerfile.sh
rm ./Python2.7.18_toBeInstalled/*.txt
rm ./Python2.7.18_toBeInstalled/*.log
rm ./Python2.7.18_toBeInstalled/*.7z*
rm -r ./Python2.7.18_toBeInstalled/packages
rm ./Python2.7.18_toBeInstalled/listForDockerfile.sh
rm ./Python2.7.18_toBeInstalled/configurationFile.sh
echo 'DockerFile generation is done. Locate in DockerFolder and build your final docker.\n You can remove the temporary docker with docker rmi '$1
 docker rmi -f $1
