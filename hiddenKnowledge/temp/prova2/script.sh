docker build . -t repbioinfo/jupyternotebook8
docker run -itv $HOME/JupyterFolder:/sharedFolder --volume="$HOME/.Xauthority:/root/.Xauthority:rw"  -p  8888:8888 repbioinfo/jupyternotebook8
