bioconductor=function(x){
install.packages('BiocManager', repos='http://cran.us.r-project.org')
for(y in x){
BiocManager::install(y)
}
}

cran=function(x){
for(y in x){
install.packages(x, repos='http://cran.us.r-project.org')
}
}



github=function(x){
install.packages("devtools",repos='http://cran.us.r-project.org')
dir.create("/scratch/packages")
for(y in x){
ll=capture.output(devtools::install_github(y,upgrade="always"))
url=strsplit(ll[grep("building",ll)],"[']")[[1]][2]
system(paste("find /tmp/ -type f -name ",url," | xargs mv -t /scratch/packages/",sep=""))
write(paste("trying URL '/",url,"'",sep=""), stdout())
write(paste("* installing *source* package '",tools::file_path_sans_ext(url),"' ... ",sep=""), stdout())

}
}

source("/scratch/libraryInstall.R")
