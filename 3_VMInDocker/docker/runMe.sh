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

pathSharedfoldDocks=$2/${j}_wDocker
if [ -d "$pathSharedfoldDocks" ]; then
    echo "Error: ${pathSharedfoldDocks} already exists. Use another name or delete the folder!!!!"
  exit 1
else
  echo "Installing files in ${pathSharedfoldDocks}..."

fi
mkdir $2/${j}_wDocker


retry cp -r $1/* $2/${j}_wDocker
sync
cat ./tail >> $2/${j}_wDocker/Dockerfile

