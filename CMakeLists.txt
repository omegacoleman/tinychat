﻿cmake_minimum_required(VERSION 3.1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(tinyrpc)

find_package(Boost REQUIRED COMPONENTS coroutine context thread program_options filesystem)
find_package(Protobuf REQUIRED)
find_package(Threads)
find_package(OpenSSL REQUIRED)

include(CheckCXXCompilerFlag)
include(CheckLibraryExists)

message(STATUS "Boost include: ${Boost_INCLUDE_DIRS}")

if(WIN32)
	add_definitions(-DTINYCHAT_WIN)
	set(WIN32_EXTRA_LIBRARIES ws2_32 wsock32 bcrypt)
elseif(UNIX)
	set(WIN32_EXTRA_LIBRARIES )
	add_definitions(-DTINYCHAT_UNIX)
else()
	message(FATAL_ERROR "unsupported platform")
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${OPENSSL_INCLUDE_DIR})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(CMAKE_MODULE_PATH  "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	add_definitions(-D_WIN32_WINNT=0x0601)
	add_definitions(-DBOOST_USE_WINAPI_VERSION=0x0601)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DNOMINMAX)
	add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj")
endif()

CHECK_CXX_COMPILER_FLAG(-fvisibility-inlines-hidden COMPILER_HAS_VISIBILITY_INLINE_HIDDEN)
CHECK_CXX_COMPILER_FLAG(-fvisibility=hidden COMPILER_HAS_VISIBILITY_HIDDEN)
CHECK_CXX_COMPILER_FLAG(-std=c++14 LIBCXX_HAS_STDCXX14_FLAG)
CHECK_CXX_COMPILER_FLAG(-std=c++17 LIBCXX_HAS_STDCXX17_FLAG)
CHECK_CXX_COMPILER_FLAG(-std=c++20 LIBCXX_HAS_STDCXX20_FLAG)
CHECK_CXX_COMPILER_FLAG(-fdiagnostics-color=always COMPILER_HAS_COLOR)

if (${COMPILER_HAS_VISIBILITY_INLINE_HIDDEN})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
endif()

if (${COMPILER_HAS_VISIBILITY_HIDDEN})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()

if (${COMPILER_HAS_COLOR})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
endif()

if (NOT MSVC)
	if (ANDROID)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE -pie")
	endif()
	if(LIBCXX_HAS_STDCXX20_FLAG)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O3")
		add_definitions(-DUSE_STD_STRING_VIEW)
	elseif(LIBCXX_HAS_STDCXX17_FLAG)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3")
		add_definitions(-DUSE_STD_STRING_VIEW)
	elseif(LIBCXX_HAS_STDCXX14_FLAG)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3")
	else()
		message(FATAL_ERROR "need at least GCC 5 or clang 5")
	endif()
endif(NOT MSVC)

if (MSVC)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED 17)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (WIN32)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)
endif()

add_subdirectory(tinyrpc)
include_directories(tinyrpc/include)

add_subdirectory(client)
add_subdirectory(server)

enable_testing()
add_subdirectory(test)
