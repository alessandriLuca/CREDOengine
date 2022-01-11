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


cat ./tail >> $pathSharedfoldDock/$1_rstudio/Dockerfile


cp ./configurationFile.txt $pathSharedfoldDock/$1_rstudio/
cp ./run.exe $pathSharedfoldDock/$1_rstudio/
cp ./rserver.conf $pathSharedfoldDock/$1_rstudio/
cp ./cc.R $pathSharedfoldDock/$1_rstudio/cc.R
echo "docker build . -t " $3  > $pathSharedfoldDock/$1_rstudio/script.sh
echo "tt=\$(head configurationFile.txt)" >> $pathSharedfoldDock/$1_rstudio/script.sh
echo "mkdir \$tt" >> $pathSharedfoldDock/$1_rstudio/script.sh
echo "cp ./configurationFile.txt \$tt" >> $pathSharedfoldDock/$1_rstudio/script.sh
echo "rm \$tt\id.txt" >> $pathSharedfoldDock/$1_rstudio/script.sh
echo "docker run -itv \$tt:/home/rstudio/ -v /var/run/docker.sock:/var/run/docker.sock --cidfile  \$tt\id.txt --privileged=true -p 8888:8888   -e DISABLE_AUTH=true "$3  >> $pathSharedfoldDock/$1_rstudio/script.sh
chmod +x $pathSharedfoldDock/$1_rstudio/script.sh





echo "docker build . -t " $3  > $pathSharedfoldDock/$1_rstudio/script.cmd
echo "set /p Build=<configurationFile.txt" >> $pathSharedfoldDock/$1_rstudio/script.cmd
echo "mkdir %Build%"  >> $pathSharedfoldDock/$1_rstudio/script.cmd
echo "copy configurationFile.txt %Build%"  >> $pathSharedfoldDock/$1_rstudio/script.cmd
echo "del %Build%\id.txt"  >> $pathSharedfoldDock/$1_rstudio/script.cmd
echo "docker run -itv %Build%:/home/rstudio/ -v /var/run/docker.sock:/var/run/docker.sock --privileged=true --cidfile  %Build%\id.txt -p 8888:8888   -e DISABLE_AUTH=true "$3  >> $pathSharedfoldDock/$1_rstudio/script.cmd

chmod +x $pathSharedfoldDock/$1_rstudio/script.sh
cp -r $pathSharedfoldDock/$1_rstudio/ .
rm -r $pathSharedfoldDock/$1_rstudio/
