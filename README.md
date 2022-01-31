# dockerFileGeneratorv2

In order to generate the dockerfile for a specific version of R/python and with specific libraries the instruction are pretty easy, just follow the steps : 
1) Choose R or python folder 
2) Now choose the specific version of R or python 
3) Open the configurationFile.sh with a text editor : 
    Is already present an example of how to install libraries. We will see now 4 examples, R, R with bioconductor,python2 and python3,
    a) R install.packages('Rtsne', repos='http://cran.us.r-project.org') # If you want to add other libraries simply use the command that you would use in order to install it in your local machine
    b) install.packages('BiocManager', repos='http://cran.us.r-project.org')
       BiocManager::install("GenomicRanges") # GenomicRanges needs BiocManager to be installed. As before write in this file everything that you would write from your local installation. 
     c) pip2 -v --log /scratch/pip.log install numpy
pip2 -v --log /scratch/pip.log install pandas #Python2 requires pip2, different from python3 libraries that requires pip3. Here add also other libraries copying the same structure with pip2 -v --log /scratch/pip.log install libraries. 
    d) pip3 -v --log /scratch/pip.log install pandas # As previously said, python3 requires pip3 command. The configurationFile structur for python3 is the same as the structure file for python2. 
4) Once the configurationFile.sh is modified with the chosen libraries run the runMe.sh script feeding it with two parameters, a docker temporary file name and the folder name, that will store all the necessary files for the docker file to be generated. 
5) Once the installation is finished enter in the just created folder with the folder name that you gave to the runMe.sh script. In this folder you have all the necessary files to build the docker with the specific libraries. Interesting thing is that all the libraries are already downloaded, that means that no matter what will happen in the future, broken link are no more a problem for R and python packages. Real bioinformatic Reproducibility is one step closer. 

Optional step -> Merge docker and work in a jupyter environment

- If you would like to create a docker with R and python, create two different dockerfile with the respective dockerFileFolder, using the previous step then copy the dockerFolder, in the MergeDocker folder that you can find in /dockerFileGenerator/mergeDocker/. Enter in the folder MergeDocker and follow the other instructions. 
- After the generation of the new folder, with the merged dockerfile is created you can copy this folder into the jupyter folder. Here you can run the ./runMe.sh providing 3 parameters : the mergedFolder name, the new folder name and the docker name. This process will create another dockerfile and another folder. In this folder there is a script.sh. Running this script will automatically build the docker and run the local server that host the jupyter. In order to work in this docker container with jupyter is enough to start a browser and go to localhost:8888. 
