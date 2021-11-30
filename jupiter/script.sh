 sudo docker build . -t rstudio
 sudo docker run -itd -p 2222:22 rstudio
ssh -X root@127.0.0.1 -p 2222 rstudio


