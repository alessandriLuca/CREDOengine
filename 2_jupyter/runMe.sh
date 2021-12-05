#!/bin/bash 
if [ $# -ne 3 ]
  then
    echo "You need to provide :"
        echo " The folderName of the mergedDockers  "
    echo " Folder name, for the new docker  "
    echo " DockerName to build the new docker container "
    exit
fi
mkdir ./$2


cp -r ./$1/* ./$2
echo "docker build . -t "$3 > ./$2/script.sh
echo "docker run -itv $HOME/JupyterFolder:/sharedFolder --volume="$HOME/.Xauthority:/root/.Xauthority:rw"  -p  8888:8888 "$3 >> ./$2/script.sh
#cp script.sh ./$2/

cp ./$1/Dockerfile ./$2/Dockerfile
echo 'RUN pip3 install jupyter' >> ./$2/Dockerfile
echo 'install.packages("IRkernel", repos="http://cran.us.r-project.org")' > ./$2/rscript.R
echo 'IRkernel::installspec()' >> ./$2/rscript.R
echo 'COPY rscript.R /home/' >> ./$2/Dockerfile
echo 'RUN Rscript /home/rscript.R' >> ./$2/Dockerfile
cat ./tail >> ./$2/Dockerfile
chmod +x ./$2/script.sh
