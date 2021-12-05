In each folder you can find the dockerfile for a specific version of R. You must follow the following step: 
1) Chose the specific R version. 
2) Copy that folder where you will build the docker (probably your laptop) 
3) Modify the file in toBeInstalled/ folder named "libraryInstall.R". In this file you have to write the instruction to install all the libraries (without dependencies ) required for your docker. You can follow the example and install both, bioconductor packages or cran packages. 
4) Run the script named buildMe.sh providing one argument, the docker tag as following ./buildMe.sh provaDocker . This must not be the final docker name, just a temporary docker that will be removed.
5) Now you have a new folder named dockerFolder. In this folde you can find all you needs to compile the docker.  
Thanks 
