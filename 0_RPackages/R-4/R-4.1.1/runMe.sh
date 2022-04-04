#!/bin/bash
retry() {
    until "$@"
    do
            echo "Attempt failed! Trying again"
    done
}
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
pathSharedfoldDock=$3/$2
pathSharedfoldHost=$3/$2

fi

if [ $# -eq 4 ]
then
echo "WORKS ONLY IN DOCKER CONTAINER!!!!!!!!!!!!!!!!!!!!"
pathSharedfoldDock=$3/$2
somedirpath=$(cat $4)
pathSharedfoldHost="$somedirpath"/"$( basename "$pathSharedfoldDock" )"
if ! test -f "$4"; then
    echo "ConfigurationFile does not exists. You dont have access to the dockerFileGenerator power. Run again the script for DockerFileGeneratorGUI to recreate the file."
    echo "Docker container failed!! check log"
    exit 1
fi
echo $pathSharedfoldHost

fi
if [ -d "$pathSharedfoldDock" ]; then
    echo "Error: ${pathSharedfoldDock} already exists. Use another name or delete the folder!!!!"
  exit 1
else
  echo "Installing files in ${pathSharedfoldDock}..."

fi
mkdir -p $pathSharedfoldDock

mv Dockerfile_1 Dockerfile
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
dockerTempName=$(echo "$1" | tr '[:upper:]' '[:lower:]')
if ! docker build --platform linux/amd64 . -t $dockerTempName; then
    echo "Docker container failed!! check log"
    exit 1
fi
retry cp -r . $pathSharedfoldDock
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
rm $pathSharedfoldDock/Dockerfile*
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
retry cp configurationFile.R $pathSharedfoldDock/R-4.1.1_toBeInstalled/libraryInstall.R
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
echo "Step : R library install. This might take some time."
if ! docker run --platform linux/amd64 -tv $pathSharedfoldHost/R-4.1.1_toBeInstalled:/scratch $dockerTempName /scratch/1_libraryInstall.sh; then
    echo "Docker container failed!! check log"
    exit 1
fi
mv Dockerfile Dockerfile_1
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
mv Dockerfile_2 Dockerfile
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
mv Dockerfile Dockerfile_2
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
cp Dockerfile_2 $pathSharedfoldDock/Dockerfile
sync
swapoff -a
swapon -a
printf '\n%s\n' 'Ram-cache and Swap Cleared'
echo 3 > /proc/sys/vm/drop_caches
if ! docker build --platform linux/amd64 $pathSharedfoldDock/ -t $dockerTempName; then
    echo "Docker container failed!! check log"
    exit 1
fi
echo "docker build --platform linux/amd64 . -t " $dockerTempName  > $pathSharedfoldDock/script.sh
echo 'if test -f "./configurationFile.txt"; then' >> $pathSharedfoldDock/script.sh
echo 'echo "$FILE exists."' >> $pathSharedfoldDock/script.sh
echo 'else' >> $pathSharedfoldDock/script.sh
echo 'pwd > configurationFile.txt' >> $pathSharedfoldDock/script.sh
echo 'fi' >> $pathSharedfoldDock/script.sh
echo "tt=\$(head configurationFile.txt)" >> $pathSharedfoldDock/script.sh
echo "mkdir \$tt" >> $pathSharedfoldDock/script.sh
echo "cp ./configurationFile.txt \$tt" >> $pathSharedfoldDock/script.sh
echo "rm \$tt/id.txt" >> $pathSharedfoldDock/script.sh
echo "docker run --platform linux/amd64 -itv \$tt:/sharedFolder -v /var/run/docker.sock:/var/run/docker.sock --cidfile  \$tt/id.txt "$dockerTempName  >> $pathSharedfoldDock/script.sh
chmod +x $pathSharedfoldDock/script.sh





echo "docker build --platform linux/amd64 . -t " $dockerTempName  > $pathSharedfoldDock/script.cmd
echo '@Set "Build=%CD%"' >> $pathSharedfoldDock/script.cmd
echo '@Echo(%Build%' >> $pathSharedfoldDock/script.cmd
echo '@If Not Exist "configurationFile.txt" Set /P "=%Build%" 0<NUL 1>"configurationFile.txt"' >> $pathSharedfoldDock/script.cmd

echo "mkdir %Build%"  >> $pathSharedfoldDock/script.cmd
echo "copy configurationFile.txt %Build%"  >> $pathSharedfoldDock/script.cmd
echo "del %Build%\id.txt"  >> $pathSharedfoldDock/script.cmd
echo "docker run --platform linux/amd64 -itv %Build%:/sharedFolder -v /var/run/docker.sock:/var/run/docker.sock --cidfile  %Build%\id.txt "$dockerTempName  >> $pathSharedfoldDock/script.cmd




echo 'DockerFile generation is done. Locate in DockerFolder and run script.sh or script.cmd'


