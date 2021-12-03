docker build . -t repbioinfo/jupyterdocker
docker run -itv $HOME/JupyterFolder:/sharedFolder --volume="$HOME/.Xauthority:/root/.Xauthority:rw" -e DISPLAY=$DISPLAY  --net=host repbioinfo/jupyterdocker jupyter notebook --allow-root

