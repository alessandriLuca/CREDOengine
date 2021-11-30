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

mv Dockerfile_1 Dockerfile
 docker rmi -f $1
 docker build . -t $1
cp configurationFile.R ./R-4.0.5_toBeInstalled/libraryInstall.R
 docker run -itv $(pwd)/R-4.0.5_toBeInstalled:/scratch $1 /scratch/1_libraryInstall.sh
mv Dockerfile Dockerfile_1
mv Dockerfile_2 Dockerfile
 docker build . -t $1
mv Dockerfile Dockerfile_2
mkdir $2 
cp Dockerfile_2 ./$2/Dockerfile
cp -r ./R-4.0.5 ./$2/R-4.0.5
mkdir ./$2/R-4.0.5_toBeInstalled
cp ./R-4.0.5_toBeInstalled/*.7z* ./$2/R-4.0.5_toBeInstalled/
cp ./pcre2-10.37.tar.gz ./$2/pcre2-10.37.tar.gz
cp -r ./p7zip_16.02 ./$2/
cp ./R-4.0.5_toBeInstalled/listForDockerfile.sh ./$2/R-4.0.5_toBeInstalled/listForDockerfile.sh
 docker rmi -f $1
rm -r ./R-4.0.5_toBeInstalled/packages
rm ./R-4.0.5_toBeInstalled/libraryInstall.R
rm ./R-4.0.5_toBeInstalled/nohup.out
rm ./R-4.0.5_toBeInstalled/*.7z.*
rm ./R-4.0.5_toBeInstalled/listForDockerfile.sh
rm ./R-4.0.5_toBeInstalled/toDownload.txt
rm ./R-4.0.5_toBeInstalled/toInstall.txt
