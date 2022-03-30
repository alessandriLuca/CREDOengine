#!/bin/bash 
#type here all libraries that needs to be installed


download () {
  pip3 -v --log /scratch/pip.log install $1
}

downloadgit () {
pip3 -v --log /scratch/pip.log install git+$1
a=$(pip3 download --no-deps -d /scratch/packages/ git+$1 | grep Saved |  awk '{print $2}' | xargs -l1 basename) 
echo 'a Downloading '$a ' (16.8 MB)' >> /scratch/pip.log
}

downloadConda () {
cd /anaconda/ && 7za -y x "*.7z*"
/anaconda/Anaconda3-2020.02-Linux-x86_64.sh -b -p /opt/conda 
cd ..
rm -r /anaconda 
export PATH="/opt/conda/bin/:$PATH"
conda install conda-pack -y
conda create --name snowflakes $1 ipykernel -y
mkdir /scratch/packages/
conda pack -n snowflakes -o /scratch/packages/$1.tar.gz
echo 'mkdir -p snowflakes/'$1' && tar -xzf /tmp/packages/'$1'.tar.gz -C snowflakes/'$1 >> /scratch/pip.log
}

downloadbioconda () {
cd /anaconda/ && 7za -y x "*.7z*"
/anaconda/Anaconda3-2020.02-Linux-x86_64.sh -b -p /opt/conda 
cd ..
rm -r /anaconda 
export PATH="/opt/conda/bin/:$PATH"
conda install conda-pack -y
conda create -c bioconda --name snowflakes $1 ipykernel -y
mkdir /scratch/packages/
conda pack -n snowflakes -o /scratch/packages/$1.tar.gz
echo 'mkdir -p snowflakes/'$1' && tar -xzf /tmp/packages/'$1'.tar.gz -C snowflakes/'$1 >> /scratch/pip.log
}
source /scratch/configurationFile.sh
