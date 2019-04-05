cmake_minimum_required(VERSION 3.1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(tinyrpc)

find_package(Boost REQUIRED COMPONENTS coroutine context thread)
find_package(Protobuf REQUIRED)
find_package(Threads)

include(CheckCXXCompilerFlag)
include(CheckLibraryExists)

link_libraries(${CMAKE_THREAD_LIBS_INIT})
link_libraries(${Boost_LIBRARIES})
link_libraries(${PROTOBUF_LIBRARIES})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/cmake")

add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)

if (MSVC)
	set(CompilerFlags
			CMAKE_CXX_FLAGS_DEBUG
			CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL
			CMAKE_CXX_FLAGS_RELWITHDEBINFO
			CMAKE_C_FLAGS_DEBUG
			CMAKE_C_FLAGS_RELEASE
			CMAKE_C_FLAGS_MINSIZEREL
			CMAKE_C_FLAGS_RELWITHDEBINFO
		)

	foreach(CompilerFlag ${CompilerFlags})
		string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	endforeach()

	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	add_definitions(-D_WIN32_WINNT=0x0601)
	add_definitions(-DBOOST_USE_WINAPI_VERSION=0x0601)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DNOMINMAX)
	add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj")

	if (MSVC_VERSION GREATER_EQUAL "1900")
		CHECK_CXX_COMPILER_FLAG("/std:c++latest" HAS_CPP_LATEST_FLAG)
		if (HAS_CPP_LATEST_FLAG)
			add_compile_options("/std:c++latest")
		endif()
	endif()
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

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (WIN32)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)
endif()

add_subdirectory(tinyrpc)
include_directories(${TINYRPC_INCLUDE_DIRS})

add_subdirectory(client)
add_subdirectory(server)
