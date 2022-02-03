#!/bin/bash
retry() {
    until "$@"
    do
            echo "Attempt failed! Trying again"
    done
}
#1)merged File path
#2) docker temp
#3) Final Docker name
#4) Final path
#5) path to hostPath txt file.
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
fi
if [ $# -eq 5 ]
then
echo "WORKS ONLY IN DOCKER CONTAINER!!!!!!!!!!!!!!!!!!!!"
pathSharedfoldDock=$4
pathSharedfoldHost=$(cat $5)
fi
dockerTempName=$(echo "$2" | tr '[:upper:]' '[:lower:]')
dockerName=$(echo "$3" | tr '[:upper:]' '[:lower:]')
j="$( basename "$1" )"
echo ${j}
pathSharedfoldDocks=$pathSharedfoldDock/${j}_visualStudio
if [ -d "$pathSharedfoldDocks" ]; then
    echo "Error: ${pathSharedfoldDocks} already exists. Use another name or delete the folder!!!!"
  exit 1
else
  echo "Installing files in ${pathSharedfoldDocks}..."

fi
mkdir -p $pathSharedfoldDock/${j}_visualStudio
retry cp -r $1/* $pathSharedfoldDock/${j}_visualStudio
sync
retry cp -r ./* $pathSharedfoldDock/${j}_visualStudio
sync
cat ./tail >> $pathSharedfoldDock/${j}_visualStudio/Dockerfile
echo "docker build . -t " $dockerName  > $pathSharedfoldDock/${j}_visualStudio/script.sh
echo 'if test -f "./configurationFile.txt"; then' >> $pathSharedfoldDock/${j}_visualStudio/script.sh
echo 'echo "$FILE exists."' >> $pathSharedfoldDock/${j}_visualStudio/script.sh
echo 'else' >> $pathSharedfoldDock/${j}_visualStudio/script.sh
echo 'pwd > configurationFile.txt' >> $pathSharedfoldDock/${j}_visualStudio/script.sh
echo 'fi' >> $pathSharedfoldDock/${j}_visualStudio/script.sh
echo "tt=\$(head configurationFile.txt)" >> $pathSharedfoldDock/${j}_visualStudio/script.sh
echo "mkdir \$tt" >> $pathSharedfoldDock/${j}_visualStudio/script.sh
echo "cp ./configurationFile.txt \$tt" >> $pathSharedfoldDock/${j}_visualStudio/script.sh
echo "rm \$tt/id.txt" >> $pathSharedfoldDock/${j}_visualStudio/script.sh
echo "docker run -itv \$tt:/home/visualStudio/ -v /var/run/docker.sock:/var/run/docker.sock --cidfile  \$tt/id.txt --privileged=true -p 8888:8888  -e DISABLE_AUTH=true "$dockerName  >> $pathSharedfoldDock/${j}_visualStudio/script.sh
chmod +x $pathSharedfoldDock/${j}_visualStudio/script.sh


echo "docker build . -t " $dockerName  > $pathSharedfoldDock/${j}_visualStudio/script.cmd
echo '@Set "Build=%CD%"' >> $pathSharedfoldDock/${j}_visualStudio/script.cmd
echo '@Echo(%Build%' >> $pathSharedfoldDock/${j}_visualStudio/script.cmd
echo '@If Not Exist "configurationFile.txt" Set /P "=%Build%" 0<NUL 1>"configurationFile.txt"' >> $pathSharedfoldDock/${j}_visualStudio/script.cmd
echo "mkdir %Build%"  >> $pathSharedfoldDock/${j}_visualStudio/script.cmd
echo "copy configurationFile.txt %Build%"  >> $pathSharedfoldDock/${j}_visualStudio/script.cmd
echo "del %Build%\id.txt"  >> $pathSharedfoldDock/${j}_visualStudio/script.cmd
echo "docker run -itv %Build%:/home/visualStudio/ -v /var/run/docker.sock:/var/run/docker.sock --privileged=true --cidfile  %Build%\id.txt -p 8888:8888   -e DISABLE_AUTH=true "$dockerName  >> $pathSharedfoldDock/${j}_visualStudio/script.cmd
chmod +x $pathSharedfoldDock/${j}_visualStudio/script.sh
if ! docker build $pathSharedfoldDock/${j}_visualStudio/ -t $dockerTempName; then
    echo "Docker container failed!! check log"
    exit 1
fi
echo 'DockerFile generation is done. Locate in DockerFolder and build your final docker.\n You can remove the temporary docker with docker rmi '$dockerTempName

