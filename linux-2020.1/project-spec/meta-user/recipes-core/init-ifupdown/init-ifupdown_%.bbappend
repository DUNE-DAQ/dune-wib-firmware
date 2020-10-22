SRC_URI += " \
        file://wib_interfaces \
        "
  
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
  
do_install_append() {
     install -m 0644 ${WORKDIR}/wib_interfaces ${D}${sysconfdir}/network/interfaces
}
