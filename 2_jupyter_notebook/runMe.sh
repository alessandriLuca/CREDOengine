#!/bin/bash 
if [ $# -ne 3 ]
  then
    echo "You need to provide :"
        echo " The folderName of the mergedDockers  "
    echo " a temporary docker name  "
    echo " DockerName to build the new docker container "
    exit
fi
mkdir ./$1_jupyter


cp -r ./$1/* ./$1_jupyter


cp -r RtoBeInstalled ./$1_jupyter/
docker build ./$1_jupyter -t $2
cp -r ./PtoBeInstalled ./$1_jupyter/
docker run -itv $(pwd)/$1_jupyter/PtoBeInstalled:/scratch $2 /scratch/1_libraryInstall.sh
cp $(pwd)/$1_jupyter/PtoBeInstalled/install_filesP* $(pwd)/$1_jupyter/
cp $(pwd)/$1_jupyter/PtoBeInstalled/listForDockerfileP.sh $(pwd)/$1_jupyter/
echo 'COPY install_filesP* /tmp/' >> ./$1_jupyter/Dockerfile
echo 'RUN cd /tmp/ && 7za -y x "install_filesP.7z*"' >> ./$1_jupyter/Dockerfile
echo 'COPY listForDockerfileP.sh /tmp/ ' >> ./$1_jupyter/Dockerfile
echo 'RUN /tmp/listForDockerfileP.sh ' >> ./$1_jupyter/Dockerfile
docker build ./$1_jupyter -t $2
docker run -itv $(pwd)/$1_jupyter/RtoBeInstalled:/scratch $2 /scratch/1_libraryInstall.sh

cp $(pwd)/$1_jupyter/RtoBeInstalled/install_filesR* $(pwd)/$1_jupyter/
cp $(pwd)/$1_jupyter/RtoBeInstalled/listForDockerfileR.sh $(pwd)/$1_jupyter/
echo 'COPY install_filesR* /tmp/' >> ./$1_jupyter/Dockerfile
echo 'RUN cd /tmp/ && 7za -y x "install_filesR.7z*"' >> ./$1_jupyter/Dockerfile
echo 'COPY listForDockerfileR.sh /tmp/ ' >> ./$1_jupyter/Dockerfile
echo 'RUN /tmp/listForDockerfileR.sh ' >> ./$1_jupyter/Dockerfile

docker build ./$1_jupyter -t $2
cp ./Rprofile ./$1_jupyter/

echo 'IRkernel::installspec()' >> ./$1_jupyter/rscript.R
echo 'COPY rscript.R /home/' >> ./$1_jupyter/Dockerfile
echo 'RUN Rscript /home/rscript.R' >> ./$1_jupyter/Dockerfile
echo 'COPY Rprofile /root/.Rprofile' >> ./$1_jupyter/Dockerfile

cat ./tail >> ./$1_jupyter/Dockerfile

echo "docker build . -t "$3 > ./$1_jupyter/script.sh
echo "docker run -itv $HOME/JupyterFolder:/sharedFolder --volume="$HOME/.Xauthority:/root/.Xauthority:rw"  -p  8888:8888 "$3 >> ./$1_jupyter/script.sh

chmod +x ./$1_jupyter/script.sh
rm -r /$1_jupyter/PtoBeInstalled/
rm -r /$1_jupyter/RtoBeInstalled/
