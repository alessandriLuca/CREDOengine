docker build . -t  repbioinfo/dockerfilegeneratorv3
set /p Build=<configurationFile.txt
mkdir %Build%
copy configurationFile.txt %Build%
del %Build%\id.txt
docker run -itv %Build%:/sharedFolder -v /var/run/docker.sock:/var/run/docker.sock --privileged=true --cidfile  %Build%\id.txt  -p  8888:8888 -p 3000:3000 repbioinfo/dockerfilegeneratorv3
