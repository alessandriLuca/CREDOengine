FROM library/ubuntu:20.04 as UBUNTU_BASE
MAINTAINER alessandri.luca1991@gmail.com
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get -y install build-essential zlib1g-dev libncurses5-dev libgdbm-dev libnss3-dev libssl-dev libreadline-dev libffi-dev wget \
    && apt-get -y install libblas-dev liblapack-dev \
    && apt-get -y install libatlas-base-dev \
    && apt-get -y install gfortran \
    && apt-get -y install libxml2-dev libxmlsec1-dev \
    && apt-get install -y libbz2-dev \
    && apt-get -y install pkg-config\
    && apt-get update && apt-get install -y --no-install-recommends apt-utils \
    && apt-get -y install libhdf5-dev
COPY p7zip_16.02 /tmp/
RUN apt-get update \
    && apt-get install -y wget \
    && mkdir data \
	&& apt-get -y install build-essential \
	&& cd /tmp/ && make \
	&& make all \
	&& cp ./bin/7za /bin
RUN apt-get update
RUN apt-get install -y liblzma-dev
RUN apt-get update
RUN apt-get install -y sqlite3
RUN apt-get install -y libsqlite3-dev
COPY Python-3.8.6.tgz /tmp/Python-3.8.6.tgz
RUN tar -xzvf /tmp/Python-3.8.6.tgz
RUN cd /Python-3.8.6/ && ./configure --enable-loadable-sqlite-extensions --enable-optimizations
RUN cd /Python-3.8.6/ && make
RUN cd /Python-3.8.6/ && make install
COPY pipdeptree-2.1.0-py3-none-any.whl /tmp/
RUN apt-get update \
    && apt-get install -y git
RUN pip3 install /tmp/pipdeptree-2.1.0-py3-none-any.whl
COPY Python3.8.6_toBeInstalled/install_files.7z* /tmp/
RUN cd /tmp/ && 7za -y x "*.7z*"
COPY Python3.8.6_toBeInstalled/listForDockerfile.sh /tmp/
RUN /tmp/listForDockerfile.sh
RUN apt-get update
RUN apt-get update
RUN apt-get install -y \
    apt-transport-https \
    ca-certificates \
    curl \
    gnupg \
    lsb-release

COPY ./R-4.1.1 /tmp
RUN apt-get update
RUN apt-get -y install gfortran
RUN apt-get -y install build-essential
RUN apt-get -y install fort77
RUN apt-get -y install xorg-dev
RUN apt-get -y install liblzma-dev  libblas-dev gfortran
RUN apt-get -y install gcc-multilib
RUN apt-get -y install gobjc++
RUN apt-get -y install aptitude
RUN apt-get -y install libbz2-dev
RUN apt-get -y install libpcre3-dev
RUN aptitude -y install libreadline-dev
RUN apt-get -y install libcurl4-openssl-dev
RUN apt-get -y install libcairo2-dev
RUN apt-get -y install libxt-dev
COPY pcre2-10.37.tar.gz /tmp/prec/
RUN tar -zxvf /tmp/prec/pcre2-10.37.tar.gz
RUN /pcre2-10.37/configure
RUN make -j 24
RUN make install
RUN chmod +x /tmp/configure
RUN /tmp/configure --with-x=yes --enable-R-shlib
RUN make
RUN make install
COPY p7zip_16.02 /tmp/
RUN cd /tmp/ && make
RUN chmod 777 -R /tmp/
RUN cd /tmp/ && make install
RUN rm -r /tmp/*
RUN apt install -y build-essential libcurl4-gnutls-dev libxml2-dev libssl-dev
RUN apt-get install -y libcurl4-openssl-dev libssl-dev
RUN apt-get install -y wget
COPY R-4.1.1_toBeInstalled/install_files.7z* /tmp/
RUN cd /tmp/ && 7za -y x "*.7z*" 
COPY R-4.1.1_toBeInstalled/listForDockerfile.sh /tmp/
RUN /tmp/listForDockerfile.sh
COPY RtoBeInstalled/install_filesR* /tmp/
RUN cd /tmp/ && 7za -y x "install_filesR.7z*"
COPY RtoBeInstalled/listForDockerfileR.sh /tmp/
RUN /tmp/listForDockerfileR.sh

COPY rstudioServer /rstudioServer
RUN cd /rstudioServer/ && 7za -y x "*.7z*"
RUN apt-get install -y psmisc libapparmor1 sudo libclang-dev libpq5
RUN dpkg -i /rstudioServer/rstudio-server-2021.09.1-372-amd64.deb
#CMD ["/usr/lib/rstudio-server/bin/rserver", "--server-daemonize", "0"]
RUN ln -s /usr/lib/rstudio-server/bin/pandoc/pandoc /usr/local/bin \
  && ln -s /usr/lib/rstudio-server/bin/pandoc/pandoc-citeproc /usr/local/bin \
  && mkdir -p /etc/R \
  && echo '\n\
    \n# Configure httr to perform out-of-band authentication if HTTR_LOCALHOST \
    \n# is not set since a redirect to localhost may not work depending upon \
    \n# where this Docker container is running. \
    \nif(is.na(Sys.getenv("HTTR_LOCALHOST", unset=NA))) { \
    \n  options(httr_oob_default = TRUE) \
    \n}' >> /usr/local/lib/R/etc/Rprofile.site \
  && echo "PATH=${PATH}" >> /usr/local/lib/R/etc/Renviron \
  && useradd rstudio \
  && echo "rstudio:rstudio" | chpasswd \
	&& mkdir /home/rstudio \
	&& chown rstudio:rstudio /home/rstudio \
	&& addgroup rstudio staff \
  &&  echo 'rsession-which-r=/usr/local/bin/R' >> /etc/rstudio/rserver.conf \
  && echo 'lock-type=advisory' >> /etc/rstudio/file-locks \
COPY configs/userconf.sh /etc/cont-init.d/userconf
COPY configs/add_shiny.sh /etc/cont-init.d/add
COPY configs/disable_auth_rserver.conf /etc/rstudio/disable_auth_rserver.conf
COPY configs/pam-helper.sh /usr/lib/rstudio-server/bin/pam-helper

EXPOSE 8787

VOLUME /home/rstudio/kitematic
ENV USER=rstudio
RUN apt-get install -y python3-dev
CMD ["/usr/lib/rstudio-server/bin/rserver", "--server-daemonize", "0", "--auth-none", "1"]
