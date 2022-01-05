#!/bin/bash 
if [ $# -ne 4 ] &&  [ $# -ne 5 ]
  then
    echo "You need to provide :"
        echo " The folderName of the mergedDockers  "
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
mkdir $pathSharedfoldDock
fi

if [ $# -eq 5 ]
then
echo "WORKS ONLY IN DOCKER CONTAINER!!!!!!!!!!!!!!!!!!!!"
pathSharedfoldDock=$4
somedirpath=$(cat $5)
pathSharedfoldHost="$somedirpath"/"$( basename "$pathSharedfoldDock" )"
echo $pathSharedfoldHost
mkdir $pathSharedfoldDock

fi



#folderName,dockerName,finalDockerName,temporaryFolderPath,hostPath

mkdir $pathSharedfoldDock/$1_rstudio
cp -r ./$1/* $pathSharedfoldDock/$1_rstudio
cp -r ./rstudioServer $pathSharedfoldDock/$1_rstudio/
cp -r ./configs $pathSharedfoldDock/$1_rstudio/

cp -r RtoBeInstalled $pathSharedfoldDock/$1_rstudio/
docker build $pathSharedfoldDock/$1_rstudio -t $2
docker run -v $pathSharedfoldHost/$1_rstudio/RtoBeInstalled:/scratch $2 /scratch/1_libraryInstall.sh
echo 'COPY RtoBeInstalled/install_filesR* /tmp/' >> $pathSharedfoldDock/$1_rstudio/Dockerfile
echo 'RUN cd /tmp/ && 7za -y x "install_filesR.7z*"' >> $pathSharedfoldDock/$1_rstudio/Dockerfile
echo 'COPY RtoBeInstalled/listForDockerfileR.sh /tmp/ ' >> $pathSharedfoldDock/$1_rstudio/Dockerfile
echo 'RUN /tmp/listForDockerfileR.sh ' >> $pathSharedfoldDock/$1_rstudio/Dockerfile




cat ./tail >> $pathSharedfoldDock/$1_rstudio/Dockerfile
echo "docker run  -p 127.0.0.1:8787:8787   -e DISABLE_AUTH=true  "$3 > $pathSharedfoldDock/$1_rstudio/script.sh
chmod +x $pathSharedfoldDock/$1_rstudio/script.sh
