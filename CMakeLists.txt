# -----------------------------------------------------
#   DECLARE THE PROJECT

# minimum version of CMake that can parse this file
cmake_minimum_required(VERSION 2.8.12...3.19.1)

# By default, project configuration will be Release
# (must be done before project() statement)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# Declare the project
project("vircon32-libretro" LANGUAGES C CXX)

# Set name for final executable
set(CORE_BINARY_NAME "vircon32_libretro")

# -----------------------------------------------------
#   FINDING PROJECT DEPENDENCIES

# Try to find OpenGL along with everything it needs
find_package(OpenGL REQUIRED)

# Report whether it was found
if(OPENGL_FOUND OR OPENGL_LIBRARY OR OPENGL_BUNDLED)
    message(STATUS "* OPENGL found")
else()
    message(SEND_ERROR "* OPENGL not found")
endif()

# -----------------------------------------------------
#   DEFINE PROJECT STRUCTURE

# Define folders where compiler should look for includes
include_directories(${OPENGL_INCLUDE_DIR} OpenGLHeaders)
include_directories(.)

# On ARM systems (typically phones, RPi, or embedded devices)
# use OpenGL ES; for other systems just use OpenGL core profile
if(ENABLE_OPENGLES2)
    message(STATUS "Using OpenGL ES 2")
    set(GLSYM_SRC glsym/rglgen.c glsym/glsym_es2.c)
elseif(ENABLE_OPENGLES3)
    message(STATUS "Using OpenGL ES 3")
    set(GLSYM_SRC glsym/rglgen.c glsym/glsym_es3.c)
else()
    message(STATUS "Using OpenGL Core profile")
    set(GLSYM_SRC glsym/rglgen.c glsym/glsym_gl.c)
endif()

# Define the Vircon32 logic library, in the subfolder
add_subdirectory(ConsoleLogic)

# -----------------------------------------------------
#   DECLARE TARGET BINARIES

# Define final library for the core with its sources
add_library(${CORE_BINARY_NAME} SHARED
    Globals.cpp
    libretro.cpp
    Logging.cpp
    VideoOutput.cpp
    ${CONSOLE_LOGIC_SRC}
    ${GLSYM_SRC})

# Our C++ sources need C++11 to compile
set_property(TARGET ${CORE_BINARY_NAME} PROPERTY CXX_STANDARD 11)

# The code needs this preprocessor variable
if(ENABLE_OPENGLES2)
    target_compile_definitions(${CORE_BINARY_NAME} PUBLIC HAVE_OPENGLES2=1)
elseif(ENABLE_OPENGLES3)
    target_compile_definitions(${CORE_BINARY_NAME} PUBLIC HAVE_OPENGLES3=1)
endif()

# Libraries to link to the core
target_link_libraries(${CORE_BINARY_NAME}
    V32ConsoleLogic
    ${OPENGL_LIBRARIES})

# For gcc/MinGW we need to have the C/C++ std libs as part of the final library
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${CORE_BINARY_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()

# Disable any unwanted prefix for the final library
set_target_properties(${CORE_BINARY_NAME} PROPERTIES PREFIX "")
