# dockerFileGenerator
The Docker engine opened the way to reproducibility in the bioinformatics arena. Furthermore, docker containerization is getting very common as tools to facilitate the distribution of  bioinformatics workflows. Although, it is now very much spread in the biological community because of the complexity of the steps required to build from scratch a docker container. Please use the GUI if you prefer a more automatic way and easy way to generate docker and dockerfiles https://github.com/alessandriLuca/dockerFileGeneratorGUI.
The docker customization will be achieved through the following steps:   \\
Selecting a specific Python version \\
Selecting a specific R version \\
Defining the list of python/R libraries to be included in the docker image \\
Selecting a specific GUI (jupyterlab, visual studio or Rstudio)\\
Selecting the option of executing docker/singularity instances in docker \\

DockerFileGenerator can also be used using only the command line. With this approach you can generate dockerfile and docker container step by step and with more personalization.  DockerFileGenerator command line works only on linux systems or OSX.\\
Layer_0\\
Before running the main script (runMe.sh) is important to modify the configurationFile.R/sh respectively for R layer and python layer. As explained before, this file contains all the instruction for library installation : \\
Python: \\
download libraryName : this is the classic pip installation. Example is download numpy \\
downloadgit : download and install library using git. Example is downloadgit https://github.com/httpie/httpie \\
downloadConda (Only for python >= 3.0.0 ): download conda environment and install it. Example is downloadConda biopython. Conda Environment will be stored in /snowflakes/condaName folder of the docker container and to activate it is enough to run the following code
source /snowflakes/condapackageName/bin/activate\\

 
downloadbioconda (Only for python >= 3.0.0 ): download conda environment and install it. Example is downloadbioconda mageck. Conda Environment will be stored in /snowflakes/condaName folder of the docker container and to activate it is enough to run the following coder
source /snowflakes/condapackageName/bin/activate \\

 
R: \\
bioconductor : install libraries that require bioconductor. Example is bioconductor("GenomicRanges") \\
cran : install classic libraries from cran repositories. Example is cran("Rtsne") \\
github : install libraries from github. Example is github("kendomaniac/rCASC") \\
After this file is completed and saved the runMe.sh file can be run. In each layer there is an example.sh file that shows how to run it. \\ These are the parameters :  \\
Temporary docker name. This name will be used for the dummy docker container. Be sure this name is not already taken from an important container or it will overwrite the existing one. \\
Result folder name. \\
Absolute path of the folder in which all the results will be stored.  \\
Absolute path to the configurationFile.txt.  ConfigurationFile.txt must contain the absolute path to the host folder (third parameter). This parameter is optional and needed only if you are running dockerFileGenerator in a docker container.  Do not pass a fourth input argument if you are running dockerFileGenerator on a local machine.  \\

Layer_1 \\
This layer merges two dockerFolder and dockerFile in one.  \\
NB Do not merge two R dockers. \\
The following are the input parameter for the runMe.sh script :  \\
Temporary docker name. This name will be used for the dummy docker container. Be sure this name is not already taken from an important container or it will overwrite the existing one.  \\
absolute path of the first dockerFolder \\
absolute path of the second dockerFolder \\
Result folder name \\
Layer_2 \\
This layer will add a GUI to the docker. This GUI will be accessible on localhost:8888 once the docker is built and running. The following are the parameters for runMe.sh script :  \\
absolute path for the merged folder (Layer_1 results). NB. For jupyter is required python and R installation, so the merged folder. For Rstudio and visual studio it can be provided just a python installation or R installation (Layer_0 single result). \\
dummy docker name  \\
Final docker name \\
Absolute path of the folder in which all the results will be stored.  \\

Layer_3 \\
This layer will add a virtual engine to the container choosing between docker or singularity.  \\
The following are the required input parameter to the runMe.sh script :  \\
Absolute path to a dockerFolder generated from dockerFileGenerator Layer_2.  \\
 Absolute path of the folder in which all the results will be stored.  \\
