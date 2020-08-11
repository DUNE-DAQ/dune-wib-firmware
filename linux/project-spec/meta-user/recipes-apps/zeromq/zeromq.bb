SUMMARY = "ZeroMQ"
LICENSE = "LGPLv3+"
SECTION = "PETALINUX/apps"

SRC_URI = "https://github.com/zeromq/libzmq/releases/download/v4.3.2/zeromq-4.3.2.tar.gz "

S = "${WORKDIR}/zeromq-4.3.2"

inherit autotools pkgconfig

