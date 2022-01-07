docker build . -t rstudio
docker run  -p 127.0.0.1:8787:8787   -e DISABLE_AUTH=true  rstudio
