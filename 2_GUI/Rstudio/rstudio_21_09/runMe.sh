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
fi
if [ $# -eq 5 ]
then
echo "WORKS ONLY IN DOCKER CONTAINER!!!!!!!!!!!!!!!!!!!!"
pathSharedfoldDock=$4
pathSharedfoldHost=$(cat $5)
if ! test -f "$4"; then
    echo "ConfigurationFile does not exists. You dont have access to the dockerFileGenerator power. Run again the script for DockerFileGeneratorGUI to recreate the file."
    echo "Docker container failed!! check log"
    exit 1
fi
echo $pathSharedfoldHost
fi
dockerTempName=$(echo "$2" | tr '[:upper:]' '[:lower:]')
dockerName=$(echo "$3" | tr '[:upper:]' '[:lower:]')
#folderName,dockerName,finalDockerName,temporaryFolderPath,hostPath
j="$( basename "$1" )"
echo ${j}
pathSharedfoldDocks=$pathSharedfoldDock/${j}_rstudio_21_09
if [ -d "$pathSharedfoldDocks" ]; then
    echo "Error: ${pathSharedfoldDocks} already exists. Use another name or delete the folder!!!!"
  exit 1
else
  echo "Installing files in ${pathSharedfoldDocks}..."

fi
mkdir -p $pathSharedfoldDock/${j}_rstudio_21_09
retry cp -r $1/* $pathSharedfoldDock/${j}_rstudio_21_09
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
retry cp -r ./* $pathSharedfoldDock/${j}_rstudio_21_09
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
cat ./tail >> $pathSharedfoldDock/${j}_rstudio_21_09/Dockerfile
echo "docker build --platform linux/amd64 . -t " $dockerName  > $pathSharedfoldDock/${j}_rstudio_21_09/script.sh
echo 'if test -f "./configurationFile.txt"; then' >> $pathSharedfoldDock/${j}_rstudio_21_09/script.sh
echo 'echo "$FILE exists."' >> $pathSharedfoldDock/${j}_rstudio_21_09/script.sh
echo 'else' >> $pathSharedfoldDock/${j}_rstudio_21_09/script.sh
echo 'pwd > configurationFile.txt' >> $pathSharedfoldDock/${j}_rstudio_21_09/script.sh
echo 'fi' >> $pathSharedfoldDock/${j}_rstudio_21_09/script.sh
echo "tt=\$(head configurationFile.txt)" >> $pathSharedfoldDock/${j}_rstudio_21_09/script.sh
echo "mkdir \$tt" >> $pathSharedfoldDock/${j}_rstudio_21_09/script.sh
echo "cp ./configurationFile.txt \$tt" >> $pathSharedfoldDock/${j}_rstudio_21_09/script.sh
echo "rm \$tt/id.txt" >> $pathSharedfoldDock/${j}_rstudio_21_09/script.sh
echo "docker run --platform linux/amd64 -itv \$tt:/home/rstudio/ -v /var/run/docker.sock:/var/run/docker.sock --cidfile  \$tt/id.txt --privileged=true -p 8888:8888   -e DISABLE_AUTH=true "$dockerName  >> $pathSharedfoldDock/${j}_rstudio_21_09/script.sh
chmod +x $pathSharedfoldDock/${j}_rstudio_21_09/script.sh

echo "docker build --platform linux/amd64 . -t " $dockerName  > $pathSharedfoldDock/${j}_rstudio_21_09/script.cmd
echo '@Set "Build=%CD%"' >> $pathSharedfoldDock/${j}_rstudio_21_09/script.cmd
echo '@Echo(%Build%' >> $pathSharedfoldDock/${j}_rstudio_21_09/script.cmd
echo '@If Not Exist "configurationFile.txt" Set /P "=%Build%" 0<NUL 1>"configurationFile.txt"' >> $pathSharedfoldDock/${j}_rstudio_21_09/script.cmd
echo "mkdir %Build%"  >> $pathSharedfoldDock/${j}_rstudio_21_09/script.cmd
echo "copy configurationFile.txt %Build%"  >> $pathSharedfoldDock/${j}_rstudio_21_09/script.cmd
echo "del %Build%\id.txt"  >> $pathSharedfoldDock/${j}_rstudio_21_09/script.cmd
echo "docker run --platform linux/amd64 -itv %Build%:/home/rstudio/ -v /var/run/docker.sock:/var/run/docker.sock --privileged=true --cidfile  %Build%\id.txt -p 8888:8888   -e DISABLE_AUTH=true "$dockerName  >> $pathSharedfoldDock/${j}_rstudio_21_09/script.cmd
chmod +x $pathSharedfoldDock/${j}_rstudio_21_09/script.sh

if ! docker build --platform linux/amd64 $pathSharedfoldDock/${j}_rstudio_21_09/ -t $dockerTempName; then
    echo "Docker container failed!! check log"
    exit 1
fi
echo 'DockerFile generation is done. Locate in DockerFolder and build your final docker.\n You can remove the temporary docker with docker rmi '$dockerTempName
