FROM ubuntu
RUN apt update \
    && apt install -y firefox \
                      openssh-server \
                      xauth \
    && mkdir /var/run/sshd \
    && mkdir /root/.ssh \
    && chmod 700 /root/.ssh \
    && ssh-keygen -A \
    && sed -i "s/^.*PasswordAuthentication.*$/PasswordAuthentication no/" /etc/ssh/sshd_config \
    && sed -i "s/^.*X11Forwarding.*$/X11Forwarding yes/" /etc/ssh/sshd_config \
    && sed -i "s/^.*X11UseLocalhost.*$/X11UseLocalhost no/" /etc/ssh/sshd_config \
    && grep "^X11UseLocalhost" /etc/ssh/sshd_config || echo "X11UseLocalhost no" >> /etc/ssh/sshd_config
RUN apt-get update
RUN apt -y --fix-broken install
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
RUN apt-get -y install libnss3
RUN apt-get -y install libclang-dev
RUN apt-get -y install libxkbcommon-x11-0
RUN apt-get -y install libpq5
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

COPY pcre2-10.37.tar.gz /tmp/prec/
RUN tar -zxvf /tmp/prec/pcre2-10.37.tar.gz
RUN /pcre2-10.37/configure
RUN make -j 24
RUN make install
RUN chmod +x /tmp/configure
RUN /tmp/configure
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

RUN pip3 install jupyter
COPY rscript.R /home/
RUN Rscript /home/rscript.R
