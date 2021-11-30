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
RUN echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQCuAY8dtDM+/IlZBQLv4mOnw5fb2rNmLksjxm/plVm4d9KqRHaumSPlkexqxYurT83vRB3X69v70b8OMBKzFZ8O9hBiTX0xbGgWiORHcSou9n6rXoG0yw4QiGM2542tlXMAIQLiZ2czK/UHGQsPuFGdDsH7KhPX3VydpoujJjHplgpQO0xZw2oP+X6CKiXTgfg5/Di1RZUSjtkKZeoJZkN7FT1HM4WKeM9Yjl4oy6WI29HAi7sDWIDEPsbkkKDAIpUoyhVlT1al2a97c4+w+Q1//z0yF+GflErSkHSNCCHxQ96HfuO+ZWzu8clFGwq/hPPb6KTFcxBe1y2/laCOd+yU4eHWlU40BXszqw1HFM6a7S+ELScCndiWTJpwP+ciU79iA8xDhmFIfhPDz9T5v+LSdnWEhZuUSHDw8/I9jJUe0oxpjT+pMCIydemWTMwu6aZ5l5AcVE62vQcgtbQOS+eHkW3kiEJZe5FHeLjMpYGL1Wgwp50mOegglTzMDMM0eok= user@gianluca" >> /root/.ssh/authorized_keys
RUN wget https://download1.rstudio.org/desktop/bionic/amd64/rstudio-2021.09.1-372-amd64.deb
RUN dpkg -i rstudio-2021.09.1-372-amd64.deb
RUN apt-get install r-base
RUN dpkg -i rstudio-2021.09.1-372-amd64.deb

ENTRYPOINT ["sh", "-c", "/usr/sbin/sshd && tail -f /dev/null"]
