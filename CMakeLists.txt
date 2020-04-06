#   This file is part of Icecream.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

cmake_minimum_required(VERSION 3.10)

# ====================
# Version informations
# ====================
# Stable versions: x.y.z , where z < 50
# Development versions: x.y.90
# Pre-release versions: x.y.z, where z = 90 + X in rcX (1.1rc1 = 1.1.91)
set(icecream_version_major "1")
set(icecream_version_minor "3")
set(icecream_version_micro "90")

if(${icecream_version_micro} STREQUAL "")
set(icecream_version "${icecream_version_major}.${icecream_version_minor}")
else()
set(icecream_version "${icecream_version_major}.${icecream_version_minor}.${icecream_version_micro}")
endif()

set(VERSION "${icecream_version}")

# =============
# CMake init
# =============
project(icecc LANGUAGES CXX C)

SET(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/Modules
    ${CMAKE_MODULE_PATH})

if(NOT DEFINED RUNDIR)
    set(RUNDIR "/opt/icecream/var/run")
endif()
if(NOT DEFINED BINDIR)
    set(BINDIR "${CMAKE_INSTALL_PREFIX}/bin")
endif()

# ===========================
# Set up C++11 and compile flags
# ===========================
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # avoid possible mixed standard libraries

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using Clang
    # The code has numerous cast alignment warnings that only clang warns about,
    # and it probably(?) doesn't matter in practice.
    set(cast_align "")
    # Clang is not so overzealous like GCC and doesn't warn about cases which
    # are very unlikely to be problems (e.g. function argument shadowing
    # a function elsewhere), so it's not so annoying.
    set(wshadow "-Wshadow")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    # using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # using Visual Studio C++
endif()

set(CFLAGS "-g -Wall -Wextra \
    -Wshadow -Wpointer-arith ${cast_align} -Wwrite-strings \
    -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes \
    -Wnested-externs -Wundef ${CFLAGS}"
)
set(CXXFLAGS " -g -Wall -Wextra -Wpointer-arith $cast_align ${wshadow} -Wwrite-strings -Wundef ${CXXFLAGS}")

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    set(CFLAGS "${CFLAGS} -g -O0")
    set(CXXFLAGS "${CXXFLAGS} -g -O0")
else()
    set(CFLAGS "${CFLAGS} -DNDEBUG")
    set(CXXFLAGS "${CXXFLAGS} -DNDEBUG")
endif()

# ===========================
# Find required base packages
# ===========================
find_package(Libcap-ng)
find_package(LZO REQUIRED)
find_package(TAR REQUIRED)

# =============
# Documentation
# =============
find_package(DocbookToMan)


########################################################################
### Checks for header files

# Some of these are needed by popt (or other libraries included in the future).
INCLUDE(CheckIncludeFiles)
CHECK_INCLUDE_FILES("sys/signal.h;ifaddrs.h;kinfo.h;sys/param.h;devstat.h" HAVE_SIGNAL_H)
CHECK_INCLUDE_FILES("sys/socketvar.h;sys/vfs.h" HAVE_SYS_SOCKETVAR_H)
CHECK_INCLUDE_FILES("mach/host_info.h" HAVE_HOST_INFO_H)
CHECK_INCLUDE_FILES("sys/types.h;arpa/nameser.h" HAVE_ARPA_NAMESER_H)
CHECK_INCLUDE_FILES("netinet/in.h;resolv.h" HAVE_RESOLV_H)
CHECK_INCLUDE_FILES("netinet/tcp.h;netinet/tcp_var.h" HAVE_TCP_VAR_H)
CHECK_INCLUDE_FILES("sys/user.h" HAVE_USER_H)

######################################################################
### Checks for types
include(CheckTypeSize)
set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;sys/socket.h;netinet/in.h"
)
CHECK_TYPE_SIZE("struct sa_family_t" SA_FAMILY_T_SIZE)
CHECK_TYPE_SIZE("struct socklen_t" SOCKLEN_T_SIZE)
CHECK_TYPE_SIZE("struct in_port_t" IN_PORT_T_SIZE)
CHECK_TYPE_SIZE("struct in_addr_t" IN_ADDR_T_SIZE)
set(CMAKE_EXTRA_INCLUDE_FILES)

include(${CMAKE_SOURCE_DIR}/Modules/CheckStructMember.cmake)
CHECK_STRUCT_MEMBER("ifreq" "ifr_dstaddr" "net/if.h" HAVE_IFR_DSTADDR)


configure_file(cmake-config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)

include_directories(${PROJECT_BINARY_DIR})

add_subdirectory(client)
add_subdirectory(compilerwrapper)
add_subdirectory(daemon)
add_subdirectory(doc)
add_subdirectory(scheduler)
add_subdirectory(services)
add_subdirectory(suse)

#if(NOT DEFINED BUILD_EXPERIMENTAL)
#    message(SEND_ERROR "Cmake build is experimental and should not be used")
#else()
    message("The following things are known not to be finsihed/working in this build")
    message("Using this build implies you are working to fix something in this list")
    message("    Create cmake install configuration")
    message("    build on OSX")
    message("    tests")
    message("    building downstream projects (icemon)")
    message("")
    message("There are likely more")
#endif()

#[[
# ===========================
# Find required base packages
# ===========================

# unsure what these commands do
AC_C_INLINE # check for existance of inline feature? but we require C++11?
AC_DISABLE_SHARED # must static link?
AC_ENABLE_STATIC  # must static link?



########################################################################
### Checks for libraries.

# The following test taken from the cvs sources via Samba:
# If we can't find connect, try looking in -lsocket, -lnsl, and -linet.
# The Irix 5 libc.so has connect and gethostbyname, but Irix 5 also has
# libsocket.so which has a bad implementation of gethostbyname (it
# only looks in /etc/hosts), so we only look for -lsocket if we need
# it.
AC_CHECK_FUNCS(connect)
if test x"$ac_cv_func_connect" = x"no"; then
    case "$LIBS" in
    *-lnsl*) ;;
    *) AC_CHECK_LIB(nsl_s, printf) ;;
    esac
    case "$LIBS" in
    *-lnsl*) ;;
    *) AC_CHECK_LIB(nsl, printf) ;;
    esac
    case "$LIBS" in
    *-lsocket*) ;;
    *) AC_CHECK_LIB(socket, connect) ;;
    esac
    case "$LIBS" in
    *-linet*) ;;
    *) AC_CHECK_LIB(inet, connect) ;;
    esac
    dnl We can't just call AC_CHECK_FUNCS(connect) here, because the value
    dnl has been cached.
    if test x"$ac_cv_lib_socket_connect" = x"yes" || 
       test x"$ac_cv_lib_inet_connect" = x"yes"; then
        # ac_cv_func_connect=yes
        # don't!  it would cause AC_CHECK_FUNC to succeed next time configure is run
        AC_DEFINE(HAVE_CONNECT,1,[Whether the system has connect()])
    fi
fi

AC_CHECK_FUNCS([flock lockf])
AC_CHECK_FUNCS([strsignal])
AC_CHECK_FUNCS([getloadavg])

AC_CHECK_LIB(lzo2, lzo1x_1_compress, LZO_LDADD=-llzo2,
    AC_MSG_ERROR([Could not find lzo2 library - please install lzo-devel]))
AC_SUBST(LZO_LDADD)

AC_CHECK_LIB(zstd, ZSTD_compress, ZSTD_LDADD=-lzstd,
        AC_MSG_ERROR([Could not find zstd library - please install libzstd-devel]))
AC_SUBST(ZSTD_LDADD)

AC_CHECK_LIB([dl], [dlsym], [DL_LDADD=-ldl])
AC_SUBST([DL_LDADD])

AC_CHECK_HEADERS([archive.h, archive_entry.h])
AC_CHECK_LIB(archive, archive_read_data_block, ARCHIVE_LDADD=-larchive,
    AC_MSG_ERROR([Could not find libarchive library - please install libarchive-devel]))
AC_SUBST(ARCHIVE_LDADD)

AC_MSG_CHECKING([whether libarchive has archive_read_support_filter_xz()])
AC_TRY_COMPILE(
    [
    #include <archive.h>
    ],
    [
        struct archive *a;
        archive_read_support_filter_xz(a);
    ],
    [
        AC_MSG_RESULT(yes)
        AC_DEFINE(HAVE_LIBARCHIVE_XZ, 1, [Whether libarchive has archive_read_support_filter_xz()])
    ],
    [ AC_MSG_RESULT(no) ])
AC_MSG_CHECKING([whether libarchive has archive_read_support_filter_zstd()])
AC_TRY_COMPILE(
    [
    #include <archive.h>
    ],
    [
        struct archive *a;
        archive_read_support_filter_zstd(a);
    ],
    [
        AC_MSG_RESULT(yes)
        AC_DEFINE(HAVE_LIBARCHIVE_ZSTD, 1, [Whether libarchive has archive_read_support_filter_zstd()])
    ],
    [ AC_MSG_RESULT(no) ])

# In DragonFlyBSD daemon needs to be linked against libkinfo.
case $host_os in
  dragonfly*) LIB_KINFO="-lkinfo" ;;
  *) LIB_KINFO="" ;;
esac
AC_SUBST(LIB_KINFO)

AC_CHECK_PROG(CLANG,clang,clang)

AC_ARG_ENABLE(clang-rewrite-includes,
    AS_HELP_STRING([--enable-clang-rewrite-includes],
        [Use by default Clang's -frewrite-includes option.]))

if test "$enable_clang_rewrite_includes" = "yes"; then
    AC_DEFINE(HAVE_CLANG_REWRITE_INCLUDES, 1, [Define to 1 if clang supports -frewrite-includes])
elif test "$enable_clang_rewrite_includes" = "no"; then
    true  # do not enable
else
    if test -n "$CLANG"; then
        AC_MSG_CHECKING([whether clang -Werror works for unknown options])
        $CLANG -Werror -totallybogusoption -E - >/dev/null 2>/dev/null
        if test $? -eq 0; then
            AC_MSG_RESULT(no)
            # can't detect if the option is supported, but that's too old clang anyway
        else
            AC_MSG_RESULT(yes)
            AC_MSG_CHECKING([for clang -E -frewrite-includes])
            $CLANG -Werror -E -frewrite-includes - >/dev/null 2>/dev/null
            if test $? -eq 0; then
                AC_MSG_RESULT(yes)
                AC_DEFINE(HAVE_CLANG_REWRITE_INCLUDES, 1, [Define to 1 if clang supports -frewrite-includes])
            else
                AC_MSG_RESULT(no)
            fi
        fi
    fi
fi

AC_ARG_ENABLE(clang-wrappers,
    AS_HELP_STRING([--enable-clang-wrappers],
        [Use symlink wrappers for clang/clang++.]))

CLANG_SYMLINK_WRAPPERS=
if test "$enable_clang_wrappers" = "yes"; then
    CLANG_SYMLINK_WRAPPERS='clang clang++'
elif test "$enable_clang_wrappers" = "no"; then
    true # do not enable
else
    if test -n "$CLANG"; then
        CLANG_SYMLINK_WRAPPERS='clang clang++'
    fi
fi
AC_SUBST(CLANG_SYMLINK_WRAPPERS)

AC_ARG_ENABLE(gcc-color-diagnostics,
    AS_HELP_STRING([--enable-gcc-color-diagnostics],
        [Assume by default GCC has -fdiagnostics-color=auto option.]))

if test "$enable_gcc_color_diagnostics" = "yes"; then
    AC_DEFINE(HAVE_GCC_COLOR_DIAGNOSTICS, 1, [Define to 1 if gcc supports -fdiagnostics-color=auto])
else
    true  # do not enable
fi

AC_ARG_ENABLE(gcc-show-caret,
    AS_HELP_STRING([--enable-gcc-show-caret],
        [Assume by default GCC has -fdiagnostics-show-caret option.]))

if test "$enable_gcc_show_caret" = "yes"; then
    AC_DEFINE(HAVE_GCC_SHOW_CARET, 1, [Define to 1 if gcc supports -fdiagnostics-show-caret])
elif test "$enable_gcc_show_caret" = "no"; then
    true  # do not enable
else
    AC_CHECK_PROG(GCC_BIN,gcc,gcc)
    if test -n "$GCC_BIN"; then
        AC_MSG_CHECKING([for gcc -fdiagnostics-show-caret])
        $GCC_BIN -Werror -E -fdiagnostics-show-caret - >/dev/null 2>/dev/null
        if test $? -eq 0; then
            AC_MSG_RESULT(yes)
            AC_DEFINE(HAVE_GCC_SHOW_CARET, 1, [Define to 1 if gcc supports -fdiagnostics-show-caret])
        else
            AC_MSG_RESULT(no)
        fi
    fi
fi

AC_ARG_ENABLE(gcc-fdirectives-only,
    AS_HELP_STRING([--enable-gcc-fdirectives-only],
        [Use by default GCC's -fdirectives-only option.]))

if test "$enable_gcc_fdirectives_only" = "yes"; then
    AC_DEFINE(HAVE_GCC_FDIRECTIVES_ONLY, 1, [Define to 1 if gcc supports -fdirectives-only])
elif test "$enable_gcc_fdirectives_only" = "no"; then
    true  # do not enable
else
    AC_CHECK_PROG(GCC_BIN,gcc,gcc)
    AC_MSG_CHECKING([whether $GCC_BIN -Werror works for unknown options])
    $GCC_BIN -Werror -totallybogusoption -E - >/dev/null 2>/dev/null
    if test $? -eq 0; then
        AC_MSG_RESULT(no)
        # can't detect if the option is supported, but that's too old clang anyway
    else
        AC_MSG_RESULT(yes)
        AC_MSG_CHECKING([for $GCC_BIN -E -fdirectives-only])
        $GCC_BIN -Werror -E -fdirectives-only - >/dev/null 2>/dev/null
        if test $? -eq 0; then
            AC_MSG_RESULT(yes)
            AC_DEFINE(HAVE_GCC_FDIRECTIVES_ONLY, 1, [Define to 1 if gcc supports -fdirectives-only])
        else
            AC_MSG_RESULT(no)
        fi
    fi
fi

AC_MSG_CHECKING([for -fsanitize= usage])
if echo "$CXXFLAGS" | grep -q -- -fsanitize; then
    AC_DEFINE(SANITIZER_USED, 1, [Define to 1 if compiled with -fsanitize option(s)])
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi
]]