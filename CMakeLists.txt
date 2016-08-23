# (c) Copyright 2015 Hewlett Packard Enterprise Development LP
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.

cmake_minimum_required (VERSION 2.8)

set (LEDD ops-ledd)
set (LED_PLUGINS led-plugins)
project (${LEDD})
set (SRC_DIR src)
set (INCL_DIR include)

# Define compile flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Werror")

# Rules to locate needed libraries
include(FindPkgConfig)
pkg_check_modules(CONFIG_YAML REQUIRED ops-config-yaml)
pkg_check_modules(OVSCOMMON REQUIRED libovscommon)
pkg_check_modules(OVSDB REQUIRED libovsdb)

include_directories (${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/${INCL_DIR}
                     ${OVSCOMMON_INCLUDE_DIRS}
)

configure_file(${CMAKE_SOURCE_DIR}/${INCL_DIR}/ledd_plugins.h.in
               ${CMAKE_SOURCE_DIR}/${INCL_DIR}/ledd_plugins.h)

# Sources to build ops-ledd
set (SOURCES ${SRC_DIR}/ledd.c ${SRC_DIR}/ledd_plugins.c)
# Sources to build libled-plugins
set (LIBSOURCES ${SRC_DIR}/ledd_plugins.c)

# Rules to build ops-ledd
add_executable (${LEDD} ${SOURCES})
# Rules to build libled-plugins
add_library (${LED_PLUGINS} SHARED ${LIBSOURCES})

set(LED_PLUGINS_VERSION_MAJOR "0")
set(LED_PLUGINS_VERSION_MINOR "1")
set(LED_PLUGINS_VERSION_PATCH "0")
set(LED_PLUGINS_VERSION "${LED_PLUGINS_VERSION_MAJOR}.${LED_PLUGINS_VERSION_MINOR}.${LED_PLUGINS_VERSION_PATCH}")
set_target_properties(${LED_PLUGINS} PROPERTIES VERSION ${LED_PLUGINS_VERSION})

configure_file(${SRC_DIR}/led-plugins.pc.in led-plugins.pc @ONLY)

target_link_libraries (${LEDD} ${CONFIG_YAML_LIBRARIES}
                       ${OVSCOMMON_LIBRARIES} ${OVSDB_LIBRARIES}
                       -lpthread -lrt -lsupportability)

target_link_libraries (${LED_PLUGINS}
                       ${OVSCOMMON_LIBRARIES}
                       -lpthread -lrt -lsupportability)

# Build ops-ledd cli shared libraries.
add_subdirectory(src/cli)
add_subdirectory(plugins/sysfs)
add_subdirectory(plugins/i2c)

# Rules to install ops-ledd binary in rootfs
install(TARGETS ${LEDD}
        RUNTIME DESTINATION bin)

    install(TARGETS ${LED_PLUGINS}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        )

install(FILES ${CMAKE_BINARY_DIR}/led-plugins.pc DESTINATION lib/pkgconfig)

install(FILES ${INCL_DIR}/ledd_plugins.h ${INCL_DIR}/ledd_plugin_interfaces.h
        DESTINATION include)
