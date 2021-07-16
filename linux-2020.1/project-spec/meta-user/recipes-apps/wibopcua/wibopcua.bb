SUMMARY = "WIB OPC UA Server"
LICENSE="CLOSED"
SECTION = "PETALINUX/apps"

inherit python3native
DEPENDS += "glibc boost zeromq cppzmq protobuf xerces-c xsd protobuf-native python3-native python3-jinja2-native python3-lxml-native cmake-native xsd-native indent-native"

# -compiler and -dev packages are necessary only for building on the wib itself
RDEPENDS_${PN} = "boost gdb protobuf zeromq"

#To disable hard failures from including -dev packages
INSANE_SKIP_${PN} += "dev-deps"

SRC_URI = "file://* \
          "
TARGET_CC_ARCH += "${LDFLAGS}"

#To rebuild wib sw every time
do_fetch[nostamp] = "1"
do_compile[nostamp] = "1"
do_install[nostamp] = "1"

S = "${WORKDIR}"
do_compile() {
    rm -rf build
    echo "echo 1" > nproc
    chmod +x nproc
    export PATH="`pwd`:$PATH"
    export BOOST_ROOT="${STAGING_DIR_TARGET}/usr/" 
    python3 ./quasar.py build
}
do_install() {
    install -d "${D}/opt/wibopcua/bin"
    install -m 0755 build/bin/OpcUaServer "${D}/opt/wibopcua/bin/wib_opcua"
    install -m 0755 bin/ServerConfig.xsd "${D}/opt/wibopcua/bin/ServerConfig.xsd"
    install -m 0755 bin/ServerConfig.xml "${D}/opt/wibopcua/bin/ServerConfig.xml"
    install -d "${D}/opt/wibopcua/config/defaults"
    install -m 0755 bin/config.xml "${D}/opt/wibopcua/config/defaults/config.xml"
    #The relative location of this Configuration.xsd to config.xml is important!?
    install -d "${D}/opt/wibopcua/config/Configuration"
    install -m 0755 build/Configuration/Configuration.xsd "${D}/opt/wibopcua/config/Configuration/Configuration.xsd"
    #The relative location of this Configuration.xsd to Meta.xsd is also important!? and goes up two directories!?!?
    install -d "${D}/opt/wibopcua/Meta/config/"
    install -m 0755 Meta/config/Meta.xsd "${D}/opt/wibopcua/Meta/config/Meta.xsd"
    
    
	install -d "${D}/etc/rc5.d/"
	install -m 0755 extras/opcua_init.sh "${D}/etc/rc5.d/S99opcua_init.sh"
}

FILES_${PN} += "/opt/wibopcua/*"

