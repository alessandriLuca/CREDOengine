Merge R Dockerfile with python Dockerfile.
Merge all dockerFileFolder present in this folder. Just Run the script with the dockerResult name 
$./runMe.sh bigDocker
$ cd bigDocker
$ docker build . -t bdocker
$ docker run -it bdocker




WARNING !!!!!!!! --------> Trying to merge two R version will fail. You shuold make different mergeDocker.
