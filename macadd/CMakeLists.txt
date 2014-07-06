cmake_minimum_required(VERSION 2.8)

project(maclib)

add_definitions(-Wall)

#==== Library path ====#
include_directories(SYSTEM /usr/local/include)
include_directories(SYSTEM /opt/local/include)
link_directories(/usr/local/lib)
link_directories(/opt/local/lib)

#==== OS Check ====#
if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(IS_FREEBSD true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(IS_LINUX true)
elseif(APPLE)
    set(IS_APPLE true)
endif()

#==== amd64/x86 ====#
set(Bitness 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(Bitness 64)
endif()

#==== Compiler ====#
option(UseCompileClang "Use Clang C compiler" off)

if(UseCompileClang)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()

set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install path prefix")
add_definitions(-DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

#==== Optional ====#
option(EnableAssembly "Use MMX to improve performance" on)

#==== Targets ====#
enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_COMPILER "yasm")
file(GLOB SRC_MAC4_LIB Source/MACLib/*.cpp Source/Shared/*.cpp Source/MACLib/Assembly/*.cpp)

if(EnableAssembly)
    file(GLOB SRC_MAC4_ASM Source/MACLib/Assembly/*.asm)
    set(YASM_FLAG " -m x86")
    if(Bitness EQUAL 64)
        set(YASM_FLAG " -m amd64")
    endif()
    set_source_files_properties(${SRC_MAC4_ASM}
        COMPILE_FLAGS "-f elf ${YASM_FLAG}" )

    set(MAC_FLAG " -DENABLE_ASSEMBLY -DARCH_X86")
    if(Bitness EQUAL 64)
        set(MAC_FLAG " -DENABLE_ASSEMBLY -DARCH_X86_64")
    endif()
endif()
set_source_files_properties(${SRC_MAC4_LIB}
    COMPILE_FLAGS "-O2 -fno-strict-aliasing -I${MACLIB_SOURCE_DIR}/Source/Shared -DBUILD_CROSS_PLATFORM ${MAC_FLAG}")
#don't use -O3 please!!!!!!

add_library(maclib SHARED ${SRC_MAC4_LIB} ${SRC_MAC4_ASM})

#==== Install Targets ====#
install(TARGETS maclib LIBRARY DESTINATION lib)

#install(FILES Source/Shared/config.h DESTINATION include/mac)
install(FILES Source/Shared/All.h DESTINATION include/mac)
install(FILES Source/Shared/GlobalFunctions.h DESTINATION include/mac)
install(FILES Source/Shared/ID3Genres.h DESTINATION include/mac)
install(FILES Source/Shared/IO.h DESTINATION include/mac)
install(FILES Source/Shared/SmartPtr.h DESTINATION include/mac)
install(FILES Source/Shared/StdLibFileIO.h DESTINATION include/mac)
install(FILES Source/Shared/NoWindows.h DESTINATION include/mac)
install(FILES Source/Shared/CharacterHelper.h DESTINATION include/mac)
install(FILES Source/Shared/CircleBuffer.h DESTINATION include/mac)
install(FILES Source/Shared/WindowsEnvironment.h DESTINATION include/mac)
#install(FILES Source/Shared/MACUtils.h DESTINATION include/mac)

install(FILES Source/MACLib/APECompress.h DESTINATION include/mac)
install(FILES Source/MACLib/APECompressCore.h DESTINATION include/mac)
install(FILES Source/MACLib/APECompressCreate.h DESTINATION include/mac)
install(FILES Source/MACLib/APEDecompress.h DESTINATION include/mac)
install(FILES Source/MACLib/APEHeader.h DESTINATION include/mac)
install(FILES Source/MACLib/APEInfo.h DESTINATION include/mac)
install(FILES Source/MACLib/APELink.h DESTINATION include/mac)
install(FILES Source/MACLib/APETag.h DESTINATION include/mac)
install(FILES Source/MACLib/MACLib.h DESTINATION include/mac)
install(FILES Source/MACLib/MACProgressHelper.h DESTINATION include/mac)

install(FILES maclib.pc DESTINATION lib/pkgconfig)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR "4")
set(CPACK_PACKAGE_VERSION_MINOR "11")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
set(CPACK_PACKAGE_DESCRIPTION "MAC lib")
set(CPACK_PACKAGE_CONTACT "Andrey Karpenko andrey@delfa.net")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/Debian/postinst")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andrey Karpenko") #required
 
INCLUDE(CPack)

#==== Messages ====#
message("CMake:		" ${CMAKE_VERSION})
message("OS Name:	" ${CMAKE_SYSTEM_NAME})
message("OS Version:	" ${CMAKE_SYSTEM_VERSION})
message("C Compiler:	" ${CMAKE_C_COMPILER})
message("C++ Compiler:	" ${CMAKE_CXX_COMPILER})
message("Precessor:	" ${CMAKE_SYSTEM_PROCESSOR})
message("Build type:	" ${CMAKE_BUILD_TYPE})
message("Project root:	" ${MACLIB_SOURCE_DIR})
message("Work root:	" ${PROJECT_BINARY_DIR})
message("For Arch:  " ${Bitness})
