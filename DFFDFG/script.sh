docker build . -t repbioinfo/dockerfilegenerator
docker run -itv /home/lucastormreig/JupyterFolder:/sharedFolder -v /var/run/docker.sock:/var/run/docker.sock --privileged=true --volume=/home/lucastormreig/.Xauthority:/root/.Xauthority:rw  -p  8888:8888 repbioinfo/dockerfilegenerator
