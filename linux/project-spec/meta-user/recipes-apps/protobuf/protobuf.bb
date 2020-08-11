SUMMARY = "protobuf"
SECTION = "PETALINUX/apps"
LICENSE = "BSD-3-Clause"


SRC_URI[md5sum] = "d4f6ca65aadc6310b3872ee421e79fa6"
SRC_URI[sha256sum] = "f5b3563f118f1d3d6e001705fa7082e8fc3bda50038ac3dff787650795734146"
SRC_URI = "https://github.com/google/protobuf/archive/v3.12.2.tar.gz;downloadfilename=protobuf-3.12.2.tar.gz "

EXTRA_OECONF += " --with-protoc=${STAGING_BINDIR_NATIVE}/protoc"
inherit autotools setuptools

do_compile() {
	base_do_compile
}

do_install() {
	local olddir=`pwd`

	# Install protoc compiler
	autotools_do_install

	# Install header files
	export PROTOC="${STAGING_BINDIR_NATIVE}/protoc"
	cd "${S}/${PYTHON_SRC_DIR}"
	distutils_do_install

	cd "$olddir"
}
