docker build . -t yeah3
docker run -itv /home/lucastormreig/JupyterFolder:/sharedFolder --volume=/home/lucastormreig/.Xauthority:/root/.Xauthority:rw  -p  8888:8888 yeah3
