#!/bin/bash 
if [ $# -ne 2 ]
  then
    echo "You need to provide :"
        echo " The folderName of the mergedDockers  "
                echo " The result Folder"
    exit
fi
j="$( basename "$1" )"

mkdir $2/${j}_wSingularity


cp -r $1/* $2/${j}_wSingularity
sync
cp -r ./* $2/${j}_wSingularity
sync
cat ./tail >> $2/${j}_wSingularity/Dockerfile

