#!/bin/bash
#1)merged File path
#2) docker temp
#3) Final Docker name
#4) Final path
#5) path to hostPath txt file.
retry() {
    until "$@"
    do
            echo "Attempt failed! Trying again"
    done
}
if [ $# -ne 4 ] &&  [ $# -ne 5 ]
  then
    echo "You need to provide :"
        echo " The absolute path to the mergedDockers  "
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
pathSharedfoldHost=$(cat $5)
echo $pathSharedfoldHost
mkdir $pathSharedfoldDock
fi
#folderName,dockerName,finalDockerName,temporaryFolderPath,hostPath
j="$( basename "$1" )"
echo ${j}
mkdir -p $pathSharedfoldDock/${j}_rstudio
retry cp -r $1/* $pathSharedfoldDock/${j}_rstudio
sync
retry cp -r ./* $pathSharedfoldDock/${j}_rstudio
sync
cat ./tail >> $pathSharedfoldDock/${j}_rstudio/Dockerfile
echo "docker build . -t " $3  > $pathSharedfoldDock/${j}_rstudio/script.sh
echo 'if test -f "./configurationFile.txt"; then' >> $pathSharedfoldDock/${j}_rstudio/script.sh
echo 'echo "$FILE exists."' >> $pathSharedfoldDock/${j}_rstudio/script.sh
echo 'else' >> $pathSharedfoldDock/${j}_rstudio/script.sh
echo 'pwd > configurationFile.txt' >> $pathSharedfoldDock/${j}_rstudio/script.sh
echo 'fi' >> $pathSharedfoldDock/${j}_rstudio/script.sh
echo "tt=\$(head configurationFile.txt)" >> $pathSharedfoldDock/${j}_rstudio/script.sh
echo "mkdir \$tt" >> $pathSharedfoldDock/${j}_rstudio/script.sh
echo "cp ./configurationFile.txt \$tt" >> $pathSharedfoldDock/${j}_rstudio/script.sh
echo "rm \$tt/id.txt" >> $pathSharedfoldDock/${j}_rstudio/script.sh
echo "docker run -itv \$tt:/home/rstudio/ -v /var/run/docker.sock:/var/run/docker.sock --cidfile  \$tt/id.txt --privileged=true -p 8888:8888   -e DISABLE_AUTH=true "$3  >> $pathSharedfoldDock/${j}_rstudio/script.sh
chmod +x $pathSharedfoldDock/${j}_rstudio/script.sh

echo "docker build . -t " $3  > $pathSharedfoldDock/${j}_rstudio/script.cmd
echo '@Set "Build=%CD%"' >> $pathSharedfoldDock/${j}_rstudio/script.cmd
echo '@Echo(%Build%' >> $pathSharedfoldDock/${j}_rstudio/script.cmd
echo '@If Not Exist "configurationFile.txt" Set /P "=%Build%" 0<NUL 1>"configurationFile.txt"' >> $pathSharedfoldDock/${j}_rstudio/script.cmd
echo "mkdir %Build%"  >> $pathSharedfoldDock/${j}_rstudio/script.cmd
echo "copy configurationFile.txt %Build%"  >> $pathSharedfoldDock/${j}_rstudio/script.cmd
echo "del %Build%\id.txt"  >> $pathSharedfoldDock/${j}_rstudio/script.cmd
echo "docker run -itv %Build%:/home/rstudio/ -v /var/run/docker.sock:/var/run/docker.sock --privileged=true --cidfile  %Build%\id.txt -p 8888:8888   -e DISABLE_AUTH=true "$3  >> $pathSharedfoldDock/${j}_rstudio/script.cmd
chmod +x $pathSharedfoldDock/${j}_rstudio/script.sh
