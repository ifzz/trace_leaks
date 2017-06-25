inherit pkgconfig

SUMMARY = "Memory allocation tracker"
DESCRIPTION = "Trace leaks is a debugger of memory leaks."

LICENSE = "LICENSE"
LIC_FILES_CHKSUM = "file://${LICENSE};md5=2e338baf7b9cdd872f9605ea9fffd5ae"

SRC_URI = "git://github.com/afilipov/trace_leaks.git;protocol=git;branch=master"

SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

# Global flags
OPT = "-fPIC -g -O3"
# Generate dependencies
DEP = "-MMD"

WRN = "-Wall -Wclobbered -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wsign-compare -Wtype-limits -Wuninitialized"

# Define only valid "C" optimization flags
COPT = "$(OPT) -fomit-frame-pointer -fno-stack-check"
# Define only valid "C" warnings flags
CWRN = "$(WRN) \
        -Wmissing-parameter-type -Wold-style-declaration -Wimplicit-int -Wimplicit-function-declaration -Wimplicit -Wignored-qualifiers \
        -Wformat-nonliteral -Wcast-align -Wpointer-arith -Wbad-function-cast -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations \
        -Wnested-externs -Wshadow -Wwrite-strings -Wfloat-equal -Woverride-init"

# Define only valid "C++" optimization flags
CXXOPT  = "${OPT} -std=c++11 -frtti"
# Define only valid "C" warnings flags
CXXWRN  = "$(WRN) \
        -Woverloaded-virtual \
        -Wignored-qualifiers \
        -Wformat-nonliteral -Wcast-align -Wpointer-arith -Wmissing-declarations \
        -Wcast-qual -Wwrite-strings -Wfloat-equal"

# Extra flags to give to the C++ compiler.
CXXFLAGS = "$(MCU) $(DEF) $(DEP) $(CXXOPT) $(CXXWRN)"

do_compile() {
    oe_runmake
}

do_install() {
	install -d ${D}${bindir}
	install -d ${D}${libdir}
	install -d ${D}${includedir}

	install -c -m 0755 ${S}/bin/trace_demo -D ${D}${bindir}
	install -c -m 0755 ${S}/lib/libmemtrace.a -D ${D}${libdir}
	install -c -m 0755 ${S}/lib/libmemtrace.so -D ${D}${libdir}
	install -c -m 0755 ${S}/include/* -D ${D}${includedir}
}

do_package_qa () {
}

FILES_${PN}-trace_leaks-dbg  = "${libdir}/.debug/trace_leaks.*"
FILES_${PN}-trace_leaks-dbg += "${libbin}/trace_demo"

FILES_${PN}-trace_leaks      = "${libdir}/*.so*" "${libdir}/*.a"
FILES_${PN}-trace_leaks     += "${libbin}/trace_demo"

FILES_${PN}-trace_leaks-dev  = "${libdir}/*.so* ${libdir}/*.a ${includedir}"
FILES_${PN}-trace_leaks-dev += "${libbin}/trace_demo"

# Avoid generated binaries stripping.
INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INSANE_SKIP_${PN} += "already-stripped"
