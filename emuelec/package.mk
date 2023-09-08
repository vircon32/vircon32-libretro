PKG_NAME="libretro-vircon32"
PKG_VERSION="2c609e78751c0675c2e0cd945cbc8c57356efac2"
PKG_SHA512="b407b86e4fd2f8ca2634613c922b259819d1b0aca052d963c8a5fe4b0519bb390752dbae0d52b8d28fb950cfab57e612e4c7782ec9d8ba3bf996cebb7e0f3e9c"
PKG_LICENSE="GPLv2"
PKG_SITE="https://github.com/Chandler-Kluser/vircon32-libretro"
PKG_URL="${PKG_SITE}.git"
GET_HANDLER_SUPPORT="git"
PKG_TOOLCHAIN="cmake"
PKG_DEPENDS_HOST="toolchain:host"
PKG_DEPENDS_TARGET="opengl-meson"
PKG_LONGDESC="Vircon32 32-bit Virtual Console"

pre_configure_target() {
  PKG_CMAKE_OPTS_TARGET+=" \
  -DENABLE_OPENGLES2=1 \
  -DPLATFORM=EMUELEC \
	-DOPENGL_INCLUDE_DIR=${SYSROOT_PREFIX}/usr/include \
	-DCMAKE_BUILD_TYPE=Release"
}

PKG_LIBNAME="vircon32_libretro.so"
PKG_LIBPATH="${PKG_LIBNAME}"
PKG_LIBVAR="VIRCON32_LIB"

makeinstall_target() {
  mkdir -p ${SYSROOT_PREFIX}/usr/lib/cmake/${PKG_NAME}
  cp ${PKG_LIBPATH} ${SYSROOT_PREFIX}/usr/lib/${PKG_LIBNAME}
  echo "set(${PKG_LIBVAR} ${SYSROOT_PREFIX}/usr/lib/${PKG_LIBNAME})" > ${SYSROOT_PREFIX}/usr/lib/cmake/${PKG_NAME}/${PKG_NAME}-config.cmake
}
