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



j="$( basename "$1" )"
echo ${j}
mkdir -p $pathSharedfoldDock/${j}_jupyter_notebook


retry cp -r $1/* $pathSharedfoldDock/${j}_jupyter_notebook
sync

retry cp -r ./RtoBeInstalled $pathSharedfoldDock/${j}_jupyter_notebook/
sync
docker build $pathSharedfoldDock/${j}_jupyter_notebook -t $2
retry cp -r ./PtoBeInstalled $pathSharedfoldDock/${j}_jupyter_notebook/
sync
echo "Step library Install. Might take some time. "
docker run -tv $pathSharedfoldHost/${j}_jupyter_notebook/PtoBeInstalled:/scratch $2 /scratch/1_libraryInstall.sh

echo 'COPY PtoBeInstalled/install_filesP* /tmp/' >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile
echo 'RUN cd /tmp/ && 7za -y x "install_filesP.7z*"' >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile
echo 'COPY PtoBeInstalled/listForDockerfileP.sh /tmp/ ' >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile
echo 'RUN /tmp/listForDockerfileP.sh ' >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile
docker build $pathSharedfoldDock/${j}_jupyter_notebook -t $2
echo "Step library Install. Might take some time. "
docker run -tv $pathSharedfoldHost/${j}_jupyter_notebook/RtoBeInstalled:/scratch $2 /scratch/1_libraryInstall.sh
echo 'COPY RtoBeInstalled/install_filesR* /tmp/' >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile
echo 'RUN cd /tmp/ && 7za -y x "install_filesR.7z*"' >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile
echo 'COPY RtoBeInstalled/listForDockerfileR.sh /tmp/ ' >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile
echo 'RUN /tmp/listForDockerfileR.sh ' >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile

docker build $pathSharedfoldDock/${j}_jupyter_notebook -t $2
retry cp ./Rprofile $pathSharedfoldDock/${j}_jupyter_notebook/
sync
echo 'IRkernel::installspec()' >> $pathSharedfoldDock/${j}_jupyter_notebook/rscript.R
echo 'COPY rscript.R /home/' >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile
echo 'RUN Rscript /home/rscript.R' >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile
echo 'COPY Rprofile /root/.Rprofile' >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile
echo 'ENV SHELL=/bin/bash'  >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile
cat ./tail >> $pathSharedfoldDock/${j}_jupyter_notebook/Dockerfile
sync

echo "docker build . -t " $3  > $pathSharedfoldDock/${j}_jupyter_notebook/script.sh
echo 'if test -f "./configurationFile.txt"; then' >> $pathSharedfoldDock/${j}_jupyter_notebook/script.sh
echo 'echo "$FILE exists."' >> $pathSharedfoldDock/${j}_jupyter_notebook/script.sh
echo 'else' >> $pathSharedfoldDock/${j}_jupyter_notebook/script.sh
echo 'pwd > configurationFile.txt' >> $pathSharedfoldDock/${j}_jupyter_notebook/script.sh
echo 'fi' >> $pathSharedfoldDock/${j}_jupyter_notebook/script.sh
echo "tt=\$(head configurationFile.txt)" >> $pathSharedfoldDock/${j}_jupyter_notebook/script.sh
echo "mkdir \$tt" >> $pathSharedfoldDock/${j}_jupyter_notebook/script.sh
echo "cp ./configurationFile.txt \$tt" >> $pathSharedfoldDock/${j}_jupyter_notebook/script.sh
echo "rm \$tt/id.txt" >> $pathSharedfoldDock/${j}_jupyter_notebook/script.sh
echo "docker run -itv \$tt:/sharedFolder -v /var/run/docker.sock:/var/run/docker.sock --cidfile  \$tt/id.txt --privileged=true -p  8888:8888 "$3  >> $pathSharedfoldDock/${j}_jupyter_notebook/script.sh
chmod +x $pathSharedfoldDock/${j}_jupyter_notebook/script.sh





echo "docker build . -t " $3  > $pathSharedfoldDock/${j}_jupyter_notebook/script.cmd
echo '@Set "Build=%CD%"' >> $pathSharedfoldDock/${j}_jupyter_notebook/script.cmd
echo '@Echo(%Build%' >> $pathSharedfoldDock/${j}_jupyter_notebook/script.cmd
echo '@If Not Exist "configurationFile.txt" Set /P "=%Build%" 0<NUL 1>"configurationFile.txt"' >> $pathSharedfoldDock/${j}_jupyter_notebook/script.cmd

echo "mkdir %Build%"  >> $pathSharedfoldDock/${j}_jupyter_notebook/script.cmd
echo "copy configurationFile.txt %Build%"  >> $pathSharedfoldDock/${j}_jupyter_notebook/script.cmd
echo "del %Build%\id.txt"  >> $pathSharedfoldDock/${j}_jupyter_notebook/script.cmd
echo "docker run -itv %Build%:/sharedFolder -v /var/run/docker.sock:/var/run/docker.sock --privileged=true --cidfile  %Build%\id.txt  -p  8888:8888 "$3  >> $pathSharedfoldDock/${j}_jupyter_notebook/script.cmd
