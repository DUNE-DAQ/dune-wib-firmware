SUMMARY = "WIB applications"
LICENSE="CLOSED"
SECTION = "PETALINUX/apps"

DEPENDS = "glibc zeromq cppzmq protobuf protobuf-native protobuf-c"

SRC_URI = "file://* \
          "
TARGET_CC_ARCH += "${LDFLAGS}"

S = "${WORKDIR}"
do_compile() {
        oe_runmake
}
do_install() {
        install -d ${D}/bin/
        install -m 0755 ${S}/wib_server ${D}/bin/
        install -d ${D}/etc/rc5.d/
        install -m 0755 ${S}/extras/wib_server.init ${D}/etc/rc5.d/wib_server.sh
}

