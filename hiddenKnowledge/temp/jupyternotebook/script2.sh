docker build . -t repbioinfo/jupyternotebook7
docker run -itv $HOME/JupyterFolder:/sharedFolder --volume="$HOME/.Xauthority:/root/.Xauthority:rw" --env="DISPLAY" --net=host repbioinfo/jupyternotebook7 jupyter notebook --allow-root

