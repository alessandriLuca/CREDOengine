#!/bin/bash 
if [ $# -eq 0 ]
  then
    echo "You need to provide a name for the merged Docker"
    exit
fi
V=$1
mkdir ./$2


cp -r ./$V/* ./$2
cp script.sh ./$2/

cp ./$V/Dockerfile ./$2/Dockerfile
echo 'RUN pip3 install jupyter' >> ./$2/Dockerfile
echo 'install.packages("IRkernel", repos="http://cran.us.r-project.org")' > ./$2/rscript.R
echo 'IRkernel::installspec()' >> ./$2/rscript.R
echo 'COPY rscript.R /home/' >> ./$2/Dockerfile
echo 'RUN Rscript /home/rscript.R' >> ./$2/Dockerfile
cat ./tail >> ./$2/Dockerfile
