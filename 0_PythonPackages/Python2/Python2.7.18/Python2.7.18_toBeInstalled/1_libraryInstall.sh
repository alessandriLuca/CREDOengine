#!/bin/bash 
if ! /scratch/configurationFile2.sh; then
    exit 1
fi
pipdeptree -fl > /scratch/out.txt

#SCARICO TUTTI I FILE
list=$(grep Downloading /scratch/pip.log | awk '{print $3}')
for VARIABLE in $list
do
grep $VARIABLE /scratch/pip.log | grep Looking | awk '{print $4}' | xargs -l1 wget -P /scratch/packages 
done
sed 's/[^ ]//g' /scratch/out.txt | awk '{ print length }' > /scratch/counts.txt
sorting=$(cat -n /scratch/counts.txt | sort -nk +2 -r | sed 's/\|/ /'|awk '{print $1}')

for VARIABLE in $sorting
do
a=$(grep $(awk -v c=$VARIABLE 'NR==c { print $VARIABLE }' /scratch/out.txt | sed 's/ //g' | sed -r 's/[-]+/_/g' | sed -r 's/[==]+/-/g') <<< $list)
if [[ -n "$a" ]] ; then echo 'pip install --no-dependencies /tmp/packages/'$a >> scratch/listForDockerfile.sh ; fi
done
chmod 777 /scratch/listForDockerfile.sh
if ! 7za -v25165824 a /scratch/install_files.7z /scratch/packages; then
    exit 1
fi
rm -r /scratch/packages
rm /scratch/pip.log
rm /scratch/out.txt
rm /scratch/counts.txt
chmod -R 777 /scratch/
