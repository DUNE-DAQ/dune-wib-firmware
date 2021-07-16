DESCRIPTION = "XSD Libraries for XML parsing"
SECTION = "devel"
PRIORITY = "optional"
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://xsd/LICENSE;md5=79e31466c4d9f3a85f2f987c11ebcd83"

DEPENDS = ""
DEPENDS_class-native = "xerces-c-native xsd-dev"
BBCLASSEXTEND = "native"

PROVIDES += "${PN}-dev ${PN}-dev-native"

SRC_URI = "\
	http://www.codesynthesis.com/download/xsd/4.0/xsd-4.0.0+dep.tar.bz2 \
	file://elements.patch \
	"
SRC_URI[md5sum] = "ae64d7fcd258addc9b045fe3f96208bb"
SRC_URI[sha256sum] = "eca52a9c8f52cdbe2ae4e364e4a909503493a0d51ea388fc6c9734565a859817"


S = "${WORKDIR}/xsd-4.0.0+dep"

# Per http://www.codesynthesis.com/pipermail/xsde-users/2012-October/000535.html
# Boris says to get the binary for the host, then cross-compile and install libxsd...
# However libxsd is header-only, so this is really just making the executable visible
# to the host and the headers installed on the target.

do_configure () {
	:
}
do_compile () {
	:
}
do_install () {
    install -d ${D}${includedir}
    cp -r ${S}/xsd/libxsd/xsd ${D}${includedir}/xsd
}

do_compile_class-native () {
	oe_runmake
}

do_install_class-native () {
    install -d ${D}${includedir}
    install -m 0755 -D ${S}/xsd/xsd/xsd ${D}${bindir}/xsdcxx
    cp -r ${S}/xsd/libxsd/xsd ${D}${includedir}/xsd
}
