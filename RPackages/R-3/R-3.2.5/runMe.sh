#!/bin/bash 
if [ $# -eq 0 ]
  then
    echo "You need to provide a docker tag and a dockerFolder name, for example ./buildMe.sh dockertest tempFolder"
    exit
fi

if [ $# -eq 1 ]
  then
    echo "You need to provide a docker tag and a dockerFolder name, for example ./buildMe.sh dockertest tempFolder"
    exit
fi

sudo chmod 777 R-3.2.5_toBeInstalled/1_libraryInstall.sh
mv Dockerfile_1 Dockerfile
sudo docker rmi -f $1
sudo docker build . -t $1
cp configurationFile.R ./R-3.2.5_toBeInstalled/libraryInstall.R
sudo docker run -itv $(pwd)/R-3.2.5_toBeInstalled:/scratch $1 /scratch/1_libraryInstall.sh
mv Dockerfile Dockerfile_1
mv Dockerfile_2 Dockerfile
sudo docker build . -t $1
mv Dockerfile Dockerfile_2
mkdir $2 
cp Dockerfile_2 ./$2/Dockerfile
cp -r ./R-3.2.5 ./$2/R-3.2.5
mkdir ./$2/R-3.2.5_toBeInstalled
cp ./R-3.2.5_toBeInstalled/*.7z* ./$2/R-3.2.5_toBeInstalled/
cp ./pcre2-10.37.tar.gz ./$2/pcre2-10.37.tar.gz
cp -r ./p7zip_16.02 ./$2/
cp ./R-3.2.5_toBeInstalled/listForDockerfile.sh ./$2/R-3.2.5_toBeInstalled/listForDockerfile.sh
sudo docker rmi -f $1
sudo chmod -R 777 ./*
rm -r ./R-3.2.5_toBeInstalled/packages
rm ./R-3.2.5_toBeInstalled/libraryInstall.R
rm ./R-3.2.5_toBeInstalled/nohup.out
rm ./R-3.2.5_toBeInstalled/*.7z.*
rm ./R-3.2.5_toBeInstalled/listForDockerfile.sh
rm ./R-3.2.5_toBeInstalled/toDownload.txt
rm ./R-3.2.5_toBeInstalled/toInstall.txt
