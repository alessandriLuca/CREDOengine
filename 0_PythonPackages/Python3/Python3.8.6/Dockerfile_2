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
RUN apt-get update && apt install -y libudunits2-dev libgdal-dev
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
COPY anaconda /anaconda/
COPY Python3.8.6_toBeInstalled/install_files.7z* /tmp/
RUN cd /tmp/ && 7za -y x "*.7z*"
COPY Python3.8.6_toBeInstalled/listForDockerfile.sh /tmp/
RUN /tmp/listForDockerfile.sh
