#!/bin/bash 
#type here all libraries that needs to be installed


download () {
  pip -v --log /scratch/pip.log install $1
}

downloadgit () {
pip -v --log /scratch/pip.log install git+$1
a=$(pip download --no-deps -d /scratch/packages/ git+$1 | grep Saved |  awk '{print $2}' | xargs -l1 basename) 
echo 'a Downloading '$a ' (16.8 MB)' >> /scratch/pip.log
}
source /scratch/configurationFile.sh
