CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(packetmachine)


IF(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

IF(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
ENDIF()
IF(POLICY CMP0015)
    cmake_policy(SET CMP0015 NEW)
ENDIF()



SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3 -std=c++0x")
SET(CMAKE_CXX_FLAGS_DEBUG   "-Wall -O0 -std=c++0x -g -DPACKETMACHINE_DEBUG")
SET(CMAKE_SHARED_LINKER_FLAGS "-lpcap -dynamiclib")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


INCLUDE_DIRECTORIES(${INC_DIR} ./src)
LINK_DIRECTORIES(${LIB_DIR})
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    INCLUDE_DIRECTORIES(/usr/local/include)
    LINK_DIRECTORIES(/usr/local/lib)
ENDIF()

# Build library

FILE(GLOB BASESRCS
	# Exposed code
	"src/packetmachine.cc"
	"src/property.cc"
	"src/value.cc"
	"src/config.cc"

	# Internal code
	"src/capture.cc"  "src/capture.hpp"
	"src/channel.hpp"
	"src/packet.cc"   "src/packet.hpp"
	"src/kernel.cc"   "src/kernel.hpp"
	"src/module.cc"   "src/module.hpp"
	"src/decoder.cc"  "src/decoder.hpp"
	"src/thread.cc"   "src/thread.hpp"

	# Decoder modules
	"src/modules/ether.cc"
	"src/modules/dot1q.cc"
	"src/modules/pppoe.cc"
	"src/modules/arp.cc"
	"src/modules/ipv4.cc"
	"src/modules/ipv6.cc"
	"src/modules/udp.cc"
	"src/modules/tcp.cc"
	"src/modules/icmp.cc"
	"src/modules/dns.cc"
	"src/modules/mdns.cc"
	"src/modules/dhcp.cc"
	# Decoder utilities
	"src/modules/utils.cc" "src/modules/utils.hpp"

	# Decoding utilities
	"./external/cpp-toolbox/src/buffer.cc"
	"./external/cpp-toolbox/src/buffer.hpp"
	"./external/cpp-toolbox/src/cache.hpp"
	"./external/cpp-toolbox/src/debug.hpp"
	"./external/cpp-toolbox/src/exception.hpp"
	"./external/cpp-toolbox/src/hash.cc"
	"./external/cpp-toolbox/src/hash.hpp"
)
FILE(GLOB HEADERS
  "src/packetmachine.hpp"
  "src/packetmachine/common.hpp"
  "src/packetmachine/value.hpp"
  "src/packetmachine/exception.hpp"
  "src/packetmachine/property.hpp"
  "src/packetmachine/config.hpp"
)
FILE(GLOB TESTSRCS
  "test/gtest/gtest-all.cc"	
  "test/*.cc"
  "test/modules/*.cc"
)

ADD_LIBRARY(pm-obj OBJECT ${BASESRCS} ${HEADERS})
SET_TARGET_PROPERTIES(pm-obj PROPERTIES COMPILE_FLAGS "-fPIC")

# Shared library
ADD_LIBRARY(pm-shared SHARED $<TARGET_OBJECTS:pm-obj>)
TARGET_LINK_LIBRARIES(pm-shared pcap pthread)

# Static library
ADD_LIBRARY(pm-static STATIC $<TARGET_OBJECTS:pm-obj>)
SET_PROPERTY(TARGET ${pm-static} PROPERTY POSITION_INDEPENDENT_CODE 1)
TARGET_LINK_LIBRARIES(pm-static pcap pthread)

SET_TARGET_PROPERTIES(pm-shared PROPERTIES OUTPUT_NAME packetmachine)
SET_TARGET_PROPERTIES(pm-static PROPERTIES OUTPUT_NAME packetmachine)

INSTALL(TARGETS pm-shared
  EXPORT packetmachineConfig
  INCLUDES DESTINATION include
  LIBRARY DESTINATION lib
)
INSTALL(TARGETS pm-static DESTINATION lib)

INSTALL(FILES src/packetmachine.hpp DESTINATION include/)
INSTALL(FILES ${HEADERS} DESTINATION include/packetmachine)
INSTALL(EXPORT packetmachineConfig DESTINATION cmake)

# Test binary
ADD_LIBRARY(pm-test-obj OBJECT ${TESTSRCS})
SET_PROPERTY(TARGET ${pm-test-obj} PROPERTY POSITION_INDEPENDENT_CODE 1)
ADD_EXECUTABLE(packetmachine-static-test $<TARGET_OBJECTS:pm-test-obj>)
TARGET_LINK_LIBRARIES(packetmachine-static-test pm-static)
ADD_EXECUTABLE(packetmachine-shared-test $<TARGET_OBJECTS:pm-test-obj>)
TARGET_LINK_LIBRARIES(packetmachine-shared-test pm-shared)

# Examples
ADD_EXECUTABLE(pmdump example/pmdump.cc)
TARGET_LINK_LIBRARIES(pmdump pm-static)
ADD_EXECUTABLE(pmfile example/pmfile.cc)
TARGET_LINK_LIBRARIES(pmfile pm-static)
ADD_EXECUTABLE(pmbench example/pmbench.cc)
TARGET_LINK_LIBRARIES(pmbench pm-static)
