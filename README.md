# CREDOengine (Customizable, REproducible, DOcker file generator)
# THIS VERSION OF CREDO IS A BETA. WE ARE REWRITING CREDO at the following link https://github.com/CREDOProject/
Docker is an ideal tool to guarantee reproducibility in the bioinformatics arena. Docker containerization is getting very common as tools to facilitate the distribution of  bioinformatics workflows. Although, it is now very much spread in the biological community, because of the complexity of the steps required to build from scratch a docker container. 
CREDOengine was build to facilitate the generation of docker images. 

## Layer_0  
Before running the main script (runMe.sh) it is necessary to modify the configurationFile.R/sh respectively for R layer and python layer. This file contains all the instruction for library installation:   

### Python commands to be used in config file:   
download libraryName : this is the classic pip installation. Example is **download numpy**  
downloadgit : download and install library using git. Example is **downloadgit https://github.com/httpie/httpie**   
downloadConda (only for python >= 3.0.0 ): download conda environment and install it. Example is **downloadConda biopython**. Conda Environment will be stored in /snowflakes/condaName folder of the docker container and to activate it is enough to run the following code
*source /snowflakes/condapackageName/bin/activate*  
downloadbioconda (only for python >= 3.0.0 ): download conda environment and install it. Example is **downloadbioconda mageck**. Bioconda Environment will be stored in /snowflakes/biocondaName folder of the docker container and to activate it is enough to run the following coder
*source /snowflakes/biocondapackageName/bin/activate*   

### R commands to be used in config file:   
bioconductor : install libraries that require bioconductor. Example is **bioconductor("GenomicRanges")**   
cran : install classic libraries from cran repositories. Example is **cran("Rtsne")**   
github : install libraries from github. Example is **github("kendomaniac/rCASC")**   
After this file is completed and saved the runMe.sh file can be run. 
In each layer there is an example.sh file that shows how to run it.    
These are the parameters :    
- Temporary docker name. This name will be used for the dummy docker container. Be sure this name is not already taken from an important container or it will overwrite the existing one.   
- Result folder name.   
- Absolute path of the folder in which all the results will be stored.    
- Absolute path to the configurationFile.txt.  ConfigurationFile.txt must contain the absolute path to the host folder (third parameter). This parameter is optional and needed only if you are running dockerFileGenerator in a docker container.  Do not pass a fourth input argument if you are running dockerFileGenerator on a local machine.    

## Layer_1   
This layer merges two dockerFolder and dockerFiles in one.    
IMPORTANT do not try to merge two R docker files.   
The following are the input parameter for the runMe.sh script :    
- Temporary docker name. This name will be used for the dummy docker container. Be sure this name is not already taken from an important container or it will overwrite the existing one.    
- absolute path of the first dockerFolder   
- absolute path of the second dockerFolder   
- Result folder name   

## Layer_2   
This layer will add a GUI to the docker. This GUI will be accessible on localhost:8888 once the docker is built and running. 
The following are the parameters for runMe.sh script :    
- absolute path for the merged folder (Layer_1 results). NB. For jupyter is required python and R installation, so the merged folder. For Rstudio and visual studio it can be provided just a python installation or R installation (Layer_0 single result).   
- dummy docker name    
- Final docker name   
- Absolute path of the folder in which all the results will be stored.    

## Layer_3   
This layer will add a virtual engine to the container choosing between docker or singularity.    
The following are the required input parameter to the runMe.sh script :    
- Absolute path to a dockerFolder generated from dockerFileGenerator Layer_2.    
- Absolute path of the folder in which all the results will be stored.    

## Layer_4 
This layer allows you to install additional programs using the apt package manager during the Docker file generation process. This feature enables you to incorporate specific software into your Docker environment.
To use layer 4, you need to specify the desired apt package names in the configuration file and ensure that the apt package manager is properly configured in the Dockerfile. During the Docker file generation, the specified apt packages will be downloaded and installed within the Docker image. Utilizing layer 4 enables further customization of your Docker environment by including specific programs you require for your analysis or project. Make sure to carefully follow the instructions and accurately specify the names of the necessary apt packages in the configuration file. This way, during the Docker file generation, the desired programs will be correctly installed in the Docker image.
# [CREDOgui](https://github.com/alessandriLuca/CREDOgui), the GUI to CREDOengine, provides an even more easy way to generate docker and dockerfiles.
The docker customization will be achieved through the following steps:  
Selecting a specific Python version   
Selecting a specific R version   
Defining the list of python/R libraries to be included in the docker image   
Selecting a specific GUI (jupyter, visual studio or Rstudio)  
Selecting the option of executing docker/singularity instances in docker   
Installing other software other than R or Python and using their binary files to rebuild the docker. 
