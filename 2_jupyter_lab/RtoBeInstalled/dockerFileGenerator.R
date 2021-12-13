setwd("/scratch/")
system("grep trying nohup.out > toDownload.txt")
system("cat nohup.out | grep 'installing [*]' > toInstall.txt")
toInstall=read.table("toInstall.txt")[,5]
toDownload=read.table("toDownload.txt")[,3]
if(length(grep("BiocManager",toInstall))!=0){toInstall=toInstall[-grep("BiocManager",toInstall)]}
if(length(grep("BiocManager",toDownload))!=0){toDownload=toDownload[-grep("BiocManager",toDownload)]}

dir.create("/scratch/packages")
setwd("./packages")
for(i in toDownload){
system(paste("wget ",i))
}

ll=c()
ll=append(ll,"R CMD INSTALL --build \\")

for(i in toInstall){
ll=append(ll,paste("/tmp/packages/",basename(as.vector(toDownload[grep(paste(i,"_",sep=""),toDownload)]))," \\",sep=""))
}
write(unique(ll),"/scratch/listForDockerfileR.sh")
system("7za -v25165824 a /scratch/install_filesR.7z /scratch/packages")
system("chmod 777 -R /scratch/")
