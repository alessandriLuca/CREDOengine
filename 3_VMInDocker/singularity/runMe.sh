#!/bin/bash 
retry() {
    until "$@"
    do
            echo "Attempt failed! Trying again"
    done
}
if [ $# -ne 2 ]
  then
    echo "You need to provide :"
        echo " The folderName of the mergedDockers  "
                echo " The result Folder"
    exit
fi
j="$( basename "$1" )"

mkdir $2/${j}_wSingularity


retry cp -r $1/* $2/${j}_wSingularity
sync
retry cp -r ./* $2/${j}_wSingularity
sync
cat ./tail >> $2/${j}_wSingularity/Dockerfile

