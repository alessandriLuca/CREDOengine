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

mkdir $pathSharedfoldDock/$1_jupyter_notebook


cp -r ./$1/* $pathSharedfoldDock/$1_jupyter_notebook


cp -r RtoBeInstalled $pathSharedfoldDock/$1_jupyter_notebook/
docker build $pathSharedfoldDock/$1_jupyter_notebook -t $2
cp -r ./PtoBeInstalled $pathSharedfoldDock/$1_jupyter_notebook/
docker run -tv $pathSharedfoldHost/$1_jupyter_notebook/PtoBeInstalled:/scratch $2 /scratch/1_libraryInstall.sh
#cp $(pwd)/$1_jupyter_notebook/PtoBeInstalled/install_filesP* $(pwd)/$1_jupyter_notebook/   #NOOOOOOOOOOO
#cp $(pwd)/$1_jupyter_notebook/PtoBeInstalled/listForDockerfileP.sh $(pwd)/$1_jupyter_notebook/    # NOOOOOOOOOOOOO
echo 'COPY PtoBeInstalled/install_filesP* /tmp/' >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile
echo 'RUN cd /tmp/ && 7za -y x "install_filesP.7z*"' >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile
echo 'COPY PtoBeInstalled/listForDockerfileP.sh /tmp/ ' >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile
echo 'RUN /tmp/listForDockerfileP.sh ' >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile
docker build $pathSharedfoldDock/$1_jupyter_notebook -t $2
docker run -tv $pathSharedfoldHost/$1_jupyter_notebook/RtoBeInstalled:/scratch $2 /scratch/1_libraryInstall.sh
#cp $(pwd)/$1_jupyter_notebook/RtoBeInstalled/install_filesR* $(pwd)/$1_jupyter_notebook/
#cp $(pwd)/$1_jupyter_notebook/RtoBeInstalled/listForDockerfileR.sh $(pwd)/$1_jupyter_notebook/
echo 'COPY RtoBeInstalled/install_filesR* /tmp/' >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile
echo 'RUN cd /tmp/ && 7za -y x "install_filesR.7z*"' >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile
echo 'COPY RtoBeInstalled/listForDockerfileR.sh /tmp/ ' >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile
echo 'RUN /tmp/listForDockerfileR.sh ' >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile

docker build $pathSharedfoldDock/$1_jupyter_notebook -t $2
cp ./Rprofile $pathSharedfoldDock/$1_jupyter_notebook/

echo 'IRkernel::installspec()' >> $pathSharedfoldDock/$1_jupyter_notebook/rscript.R
echo 'COPY rscript.R /home/' >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile
echo 'RUN Rscript /home/rscript.R' >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile
echo 'COPY Rprofile /root/.Rprofile' >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile
echo 'ENV SHELL=/bin/bash'  >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile
cat ./tail >> $pathSharedfoldDock/$1_jupyter_notebook/Dockerfile

cp ./configurationFile.txt $pathSharedfoldDock/$1_jupyter_notebook/
cp ./run.exe $pathSharedfoldDock/$1_jupyter_notebook/


echo "docker build . -t " $3  > $pathSharedfoldDock/$1_jupyter_notebook/script.sh
echo "tt=\$(head configurationFile.txt)" >> $pathSharedfoldDock/$1_jupyter_notebook/script.sh
echo "mkdir \$tt" >> $pathSharedfoldDock/$1_jupyter_notebook/script.sh
echo "cp ./configurationFile.txt \$tt" >> $pathSharedfoldDock/$1_jupyter_notebook/script.sh
echo "rm \$tt\id.txt" >> $pathSharedfoldDock/$1_jupyter_notebook/script.sh
echo "docker run -itv \$tt:/sharedFolder -v /var/run/docker.sock:/var/run/docker.sock --cidfile  \$tt\id.txt --privileged=true -p  8888:8888 "$3  >> $pathSharedfoldDock/$1_jupyter_notebook/script.sh
chmod +x $pathSharedfoldDock/$1_jupyter_notebook/script.sh





echo "docker build . -t " $3  > $pathSharedfoldDock/$1_jupyter_notebook/script.cmd
echo "set /p Build=<configurationFile.txt" >> $pathSharedfoldDock/$1_jupyter_notebook/script.cmd
echo "mkdir %Build%"  >> $pathSharedfoldDock/$1_jupyter_notebook/script.cmd
echo "copy configurationFile.txt %Build%"  >> $pathSharedfoldDock/$1_jupyter_notebook/script.cmd
echo "del %Build%\id.txt"  >> $pathSharedfoldDock/$1_jupyter_notebook/script.cmd
echo "docker run -itv %Build%:/sharedFolder -v /var/run/docker.sock:/var/run/docker.sock --privileged=true --cidfile  %Build%\id.txt  -p  8888:8888 "$3  >> $pathSharedfoldDock/$1_jupyter_notebook/script.cmd





rm $pathSharedfoldDock/$1_jupyter_notebook/PtoBeInstalled/*.txt
rm -r  $pathSharedfoldDock/$1_jupyter_notebook/PtoBeInstalled/packages
rm $pathSharedfoldDock/$1_jupyter_notebook/PtoBeInstalled/*.log
rm $pathSharedfoldDock/$1_jupyter_notebook/PtoBeInstalled/1_libraryInstall.sh
rm $pathSharedfoldDock/$1_jupyter_notebook/PtoBeInstalled/configurationFile.sh

rm $pathSharedfoldDock/$1_jupyter_notebook/RtoBeInstalled/*.txt
rm $pathSharedfoldDock/$1_jupyter_notebook/RtoBeInstalled/*.out
rm $pathSharedfoldDock/$1_jupyter_notebook/RtoBeInstalled/1_libraryInstall.sh
rm $pathSharedfoldDock/$1_jupyter_notebook/RtoBeInstalled/dockerFileGenerator.R
rm -r $pathSharedfoldDock/$1_jupyter_notebook/RtoBeInstalled/packages
rm $pathSharedfoldDock/$1_jupyter_notebook/RtoBeInstalled/libraryInstall.R

cp -r $pathSharedfoldDock/$1_jupyter_notebook/ .
rm -r $pathSharedfoldDock/$1_jupyter_notebook/
