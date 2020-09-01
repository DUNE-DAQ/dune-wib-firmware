FROM ubuntu:20.04

MAINTAINER Benjamin Land <bland100@sas.upenn.edu>

# build with docker build --build-arg INSTALLER=petalinux-v2020.1-final-installer.run -t petalinux:2020.1 .

#install dependences:
RUN dpkg --add-architecture i386 && apt-get update &&  DEBIAN_FRONTEND=noninteractive apt-get install -y -q \
  build-essential \
  sudo \
  tofrodos \
  iproute2 \
  gawk \
  net-tools \
  expect \
  libncurses5-dev \
  tftpd \
  update-inetd \
  libssl-dev \
  flex \
  bison \
  libselinux1 \
  gnupg \
  wget \
  socat \
  gcc-multilib \
  libsdl1.2-dev \
  libglib2.0-dev \
  lib32z1-dev \
  zlib1g:i386 \
  libgtk2.0-0 \
  screen \
  pax \
  diffstat \
  xvfb \
  xterm \
  texinfo \
  gzip \
  unzip \
  cpio \
  chrpath \
  autoconf \
  lsb-release \
  libtool \
  libtool-bin \
  locales \
  kmod \
  git \
  rsync \
  bc \
  u-boot-tools \
  libtinfo5 \
  vim \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

ARG INSTALLER

RUN locale-gen en_US.UTF-8 && update-locale

RUN adduser --disabled-password --gecos '' plnx && \
  usermod -aG sudo plnx && \
  echo "plnx ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

COPY accept-eula.sh ${INSTALLER} /

# run the install
RUN chmod a+rx /${INSTALLER} && \
  chmod a+rx /accept-eula.sh && \
  mkdir -p /opt/Xilinx && \
  chmod 777 /tmp /opt/Xilinx && \
  cd /tmp && \
  sudo -u plnx -i /accept-eula.sh /${INSTALLER} "-d /opt/Xilinx/petalinux" && \
  rm -f /${INSTALLER} /accept-eula.sh 

USER plnx
ENV HOME /home/plnx
ENV LANG en_US.UTF-8
RUN mkdir /home/plnx/project
WORKDIR /home/plnx/project

#add plnx tools to path
RUN echo "source /opt/Xilinx/petalinux/settings.sh" >> /home/plnx/.bashrc
