set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv71)
set(toolchainpath C:/build/18.0/arm/sysroots)
set(CMAKE_C_COMPILER ${toolchainpath}/i686-nilrtsdk-mingw32/usr/bin/arm-nilrt-linux-gnueabi/arm-nilrt-linux-gnueabi-gcc.exe)
set(CMAKE_CXX_COMPILER ${toolchainpath}/i686-nilrtsdk-mingw32/usr/bin/arm-nilrt-linux-gnueabi/arm-nilrt-linux-gnueabi-g++.exe)
set(CMAKE_SYSROOT ${toolchainpath}/cortexa9-vfpv3-nilrt-linux-gnueabi)
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${toolchainpath}/cortexa9-vfpv3-nilrt-linuxgnueabi/usr/include/c++/6.3.0 ${toolchainpath}/cortexa9-vfpv3-nilrt-linuxgnueabi/usr/include/c++/6.3.0/arm-nilrt-linux-gnueabi)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${toolchainpath}/cortexa9-vfpv3-nilrt-linuxgnueabi/usr/include/c++/6.3.0 ${toolchainpath}/cortexa9-vfpv3-nilrt-linuxgnueabi/usr/include/c++/6.3.0/arm-nilrt-linux-gnueabi)
set(CMAKE_C_FLAGS "-Wall -fmessage-length=0 -mfpu=vfpv3 -mfloat-abi=softfp --sysroot=${CMAKE_SYSROOT}")
set(CMAKE_CXX_FLAGS "-Wall -fmessage-length=0 -mfpu=vfpv3 -mfloat-abi=softfp --sysroot=${CMAKE_SYSROOT}")

set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

########project specific information
cmake_minimum_required(VERSION 3.14)
set(EXECUTABLE_OUTPUT_PATH bin)
set(CMAKE_BUILD_TYPE Debug) 

#########Specific information for your project
#Give your project a Name
project(myRIO_DIO) 

#Specify the C source file(s) containing main()
set(SourceFileName main.c) 

#Specify a name for the compiled executable binary 
set(ExecutableName dio) 

add_executable(${ExecutableName} ../src/${SourceFileName}) 

#in case use thread and math libraries
target_link_libraries(${ExecutableName} PRIVATE dl pthread m)

######## for project using MyRIO C Support Files
add_definitions(-DMyRio_1900 -D_GNU_SOURCE)
include_directories("../../C_Support_for_myRIO/source/" ../src/) 
file(GLOB MyRio_Support_Files "../../C_Support_for_myRIO/source/*.c" "../src/*.c")
target_sources(${ExecutableName} PRIVATE ${MyRio_Support_Files})
