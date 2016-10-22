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
	"src/object.cc"

	# Internal code
	"src/capture.cc"  "src/capture.hpp"
	"src/channel.cc"  "src/channel.hpp"
	"src/packet.cc"   "src/packet.hpp"
	"src/kernel.cc"   "src/kernel.hpp"
	"src/module.cc"   "src/module.hpp"
	"src/decoder.cc"  "src/decoder.hpp"

	# Decoder modules
	"src/modules/ether.cc"
	"src/modules/arp.cc"
	"src/modules/ipv4.cc"
	"src/modules/udp.cc"
	"src/modules/tcp.cc"
	"src/modules/tcpssn.cc"
	"src/modules/icmp.cc"

	# Decoding utilities
	"src/utils/lru.cc"    "src/utils/lru.hpp"
	"src/utils/buffer.cc" "src/utils/buffer.hpp"
	"src/utils/hash.cc"   "src/utils/hash.hpp"
)
FILE(GLOB HEADERS
  "src/packetmachine.hpp"
  "src/packetmachine/common.hpp"
  "src/packetmachine/object.hpp"
  "src/packetmachine/exception.hpp"
  "src/packetmachine/property.hpp"
)
FILE(GLOB TESTSRCS
  "test/gtest/gtest-all.cc"	
  "test/*.cc"
  "test/modules/*.cc"
)

FILE(GLOB PERFSRCS
  "perf/main.cc"
  "perf/perf.cc"
  "perf/Channel.cc"
)

ADD_LIBRARY(packetmachine SHARED ${BASESRCS} ${HEADERS})
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    TARGET_LINK_LIBRARIES(packetmachine pcap pthread rt)
ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    TARGET_LINK_LIBRARIES(packetmachine pcap pthread)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

INSTALL(TARGETS packetmachine LIBRARY DESTINATION lib)
INSTALL(FILES src/packetmachine.hpp DESTINATION include/)
INSTALL(FILES ${HEADERS} DESTINATION include/packetmachine)

# Test binary
ADD_EXECUTABLE(packetmachine-test ${TESTSRCS})
TARGET_LINK_LIBRARIES(packetmachine-test packetmachine)

# Performance check binary
ADD_EXECUTABLE(packetmachine-perf ${PERFSRCS})
TARGET_LINK_LIBRARIES(packetmachine-perf packetmachine)
