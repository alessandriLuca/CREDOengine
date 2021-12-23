docker build . -t jupjup
mkdir /home/lucastormreig/jupyterFolder
cp ./configurationFile.txt /home/lucastormreig/jupyterFolder
docker run -itv /home/lucastormreig/jupyterFolder/:/sharedFolder -p  8888:8888 jupjup
