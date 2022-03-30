#!/bin/bash
source $1/bin/activate
f=$(basename $1)
echo $f
python -m ipykernel install --name $f --display-name  "$f" 
 
