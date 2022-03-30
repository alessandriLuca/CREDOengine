#!/bin/bash
if [[ -d "/snowflakes" ]]
then
   for filename in /snowflakes/*; do
/scripts/activate.sh $filename
done

fi 
