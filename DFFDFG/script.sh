docker build . -t repbioinfo/dockerfilegeneratorv2
mkdir /home/lucastormreig/jupyterFolder
cp ./configurationFile.txt /home/lucastormreig/jupyterFolder
docker run -itv /home/lucastormreig/jupyterFolder/:/sharedFolder -v /var/run/docker.sock:/var/run/docker.sock --privileged=true -p  8888:8888 repbioinfo/dockerfilegeneratorv2
