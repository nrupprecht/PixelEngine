cmake_minimum_required(VERSION 3.21)

project(
		PixelEngine
		VERSION 0.1.0
		DESCRIPTION "A pixel game engine."
		HOMEPAGE_URL "yada yada.com"
		LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# Needed to get rid of a warning from chrono.
	#   https://stackoverflow.com/questions/76859275/error-compiling-a-cpp-containing-stdchrono-errorstatic-constexpr-unsigned-fra
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

option(METAL_CPP_BUILD_EXAMPLES "Build examples" ON)

include(FetchContent)

# Lightning logging library.
FetchContent_Declare(
		Lightning
		GIT_REPOSITORY https://github.com/nrupprecht/Lightning.git
		GIT_TAG 76aa174
)
FetchContent_MakeAvailable(Lightning)
include_directories(${Lightning_SOURCE_DIR}/include)

# Create the metal library.
add_subdirectory(metal-cmake)  # Library definition

add_library(
		PixelEngine_PixelEngine
		# Source files.
		src/pixelengine/TextureBitmap.cpp
		src/pixelengine/MetalImpl.cpp
		src/pixelengine/Utility.cpp
		src/pixelengine/application/Game.cpp
		src/pixelengine/application/AppDelegate.cpp
		src/pixelengine/input/Input.cpp
		src/pixelengine/world/World.cpp
)

add_library(PixelEngine::PixelEngine ALIAS PixelEngine_PixelEngine)

set_target_properties(
		PixelEngine_PixelEngine PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN YES
		VERSION "${PROJECT_VERSION}"
		SOVERSION "${PROJECT_VERSION_MAJOR}"
		EXPORT_NAME PixelEngine
		OUTPUT_NAME PixelEngine
)

target_compile_features(PixelEngine_PixelEngine PUBLIC cxx_std_20)

target_include_directories(
		PixelEngine_PixelEngine ${warning_guard}
		PUBLIC
		"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_link_libraries(PixelEngine_PixelEngine PUBLIC METAL_CPP)




if(METAL_CPP_BUILD_EXAMPLES)
	add_subdirectory(applications)  # Add targets
endif(METAL_CPP_BUILD_EXAMPLES)

