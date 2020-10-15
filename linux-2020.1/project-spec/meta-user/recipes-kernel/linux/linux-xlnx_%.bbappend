SRC_URI_append = " file://macb-5.4.patch"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI += " file://devtool-fragment.cfg"

