mkdir /home/lucastormreig/jupyterFolder
copy ./configurationFile.txt /home/lucastormreig/jupyterFolder
docker run -itv /home/lucastormreig/jupyterFolder/:/sharedFolder -p  8888:8888 repbioinfo/dockerfilegeneratorv2
