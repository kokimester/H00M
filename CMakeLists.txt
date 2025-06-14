#
# Specify minimum CMake version and project name
#
cmake_minimum_required (VERSION 3.24)
project (h00m)

set( CMAKE_VERBOSE_MAKEFILE on )

#
# CMake setup
#
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set (CMAKE_VERBOSE_MAKEFILE 0) # 1 should be used for debugging
set (CMAKE_SUPPRESS_REGENERATION TRUE) # Suppresses ZERO_CHECK
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
  if(NOT WIN32)
    set(GLAD_LIBRARIES dl)
  endif()
endif()

# Get colored output for ninja
set(CMAKE_COLOR_DIAGNOSTICS ON)

# This needs to be set to compile assimp
set(ASSIMP_WARNINGS_AS_ERRORS OFF)

# Optionally set things like CMAKE_CXX_STANDARD,
# CMAKE_POSITION_INDEPENDENT_CODE here
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

# This is needed for nvim clang intellisense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

# Let's ensure -std=c++xx instead of -std=g++xx
set(CMAKE_CXX_EXTENSIONS OFF)

# Let's nicely support folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


#
# GLFW options
#
option (GLFW_INSTALL OFF)
option (GLFW_BUILD_DOCS OFF)
option (GLFW_BUILD_EXAMPLES OFF)
option (GLFW_BUILD_TESTS OFF)
add_subdirectory (vendor/glfw)
add_subdirectory (vendor/assimp)
add_subdirectory (vendor/freetype)


#
# Set include paths
#
include_directories (
    src/
    src/lights/
    src/models/
    src/player/
    src/utility/
    src/rendering/
    vendor/glad/include/
    vendor/glfw/include/
    vendor/glm/
    vendor/stb/
    vendor/assimp/include/
    vendor/freetype/include/
                 )

#
# Include assimp generated config file
#
include_directories( ${CMAKE_BINARY_DIR}/vendor/assimp/include/ )

#
# Add files
#
file (GLOB         VENDORS_SOURCES vendor/glad/src/glad.c)
file (GLOB_RECURSE PROJECT_HEADERS src/*.hpp
                                   src/*.h
                                   src/lights/*.hpp
                                   src/lights/*.h
                                   src/models/*.hpp
                                   src/models/*.h
                                   src/player/*.hpp
                                   src/player/*.h
                                   src/utility/*.hpp
                                   src/utility/*.h
                                   src/rendering/*.hpp
                                   src/rendering/*.h
                                    )
file (GLOB_RECURSE PROJECT_SOURCES src/*.cpp
                                   src/*.c
                                   src/lights/*.cpp
                                   src/lights/*.c
                                   src/models/*.cpp
                                   src/models/*.c
                                   src/player/*.cpp
                                   src/player/*.c
                                   src/utility/*.cpp
                                   src/utility/*.c
                                   src/rendering/*.cpp
                                   src/rendering/*.c
                                    )
file (GLOB_RECURSE PROJECT_SHADERS shaders/*.comp
                                   shaders/*.frag
                                   shaders/*.geom
                                   shaders/*.vert)
file (GLOB         PROJECT_CONFIGS CMakeLists.txt
                                   README.rst
                                  .gitignore
                                  .gitmodules)

#
# Organizing files
#
source_group ("headers" FILES ${PROJECT_HEADERS})
source_group ("shaders" FILES ${PROJECT_SHADERS})
source_group ("sources" FILES ${PROJECT_SOURCES})
source_group ("vendors" FILES ${VENDORS_SOURCES})

#
# Set executable and target link libraries
#
add_definitions (-DGLFW_INCLUDE_NONE
                 -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
add_executable (${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS}
                                ${PROJECT_SHADERS} ${PROJECT_CONFIGS}
                                ${VENDORS_SOURCES})
target_link_libraries (${PROJECT_NAME}
                       glfw
                       ${GLFW_LIBRARIES}
                       ${GLAD_LIBRARIES}
                       assimp
                       freetype)
set_target_properties (${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
