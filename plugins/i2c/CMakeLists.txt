# Copyright Mellanox Technologies, Ltd. 2001-2016.
# This software product is licensed under Apache version 2, as detailed in
# the COPYING file.

cmake_minimum_required (VERSION 2.8)

set (LEDD_PLUGIN ledd-i2c-plugin)
project (${LEDD_PLUGIN})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Werror")

# Rules to locate needed libraries
include(FindPkgConfig)
pkg_check_modules(OVSCOMMON REQUIRED libovscommon)

include_directories (${PROJECT_BINARY_DIR}
                     ${OVSCOMMON_INCLUDE_DIRS}
)

# Source files to build i2c ledd plugin
set (SOURCES i2c-plugin.c)

# Rules to build ledd plugin
add_library (${LEDD_PLUGIN} SHARED ${SOURCES})

target_link_libraries (${LEDD_PLUGIN}
                       ${CONFIG_YAML_LIBRARIES}
                       ${OVSCOMMON_LIBRARIES}
)

# Rules to install ops-ledd binary in rootfs
install(TARGETS ${LEDD_PLUGIN}
        LIBRARY DESTINATION lib/platform/plugins
)
