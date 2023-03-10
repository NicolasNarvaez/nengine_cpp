##################################################################################
############################## project, general settings
##################################################################################
cmake_minimum_required (VERSION 3.2)
project (NEngine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# FLAGS
# set CMAKE_BUILD_TYPE=Debug to debug
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -D DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -D NDEBUG")

SET(GCC_COVERAGE_LINK_FLAGS "-lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi")
SET(CMAKE_EXE_LINKER_FLAGS	"${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

##################################################################################
############################## preprocesor defines
##################################################################################
add_compile_definitions(NENGINE_LLRENDERER_VENDOR_VULKAN)

##################################################################################
############################## targets, sources
##################################################################################

add_executable(main src/main.cpp)
# set(CMAKE_CURRENT_SOURCE_DIR src/Core)
file(GLOB_RECURSE headers src/Core/*.hpp)
file(GLOB_RECURSE sources src/Core/*.cpp)
# file(GLOB_RECURSE sources src/Core/*.cpp src/Core/*.cppm)

foreach(source IN LISTS sources)
  get_filename_component(filename ${source} NAME_WLE)
  get_filename_component(directory ${source} DIRECTORY)
  string(REGEX REPLACE "^.*src/Core/" "" directory ${directory})
  string(REGEX REPLACE "/" "_" directory ${directory})

  if(${source} MATCHES ".cppm$")
    get_filename_component(basename ${source} NAME_WE)
    set(object_file ${CMAKE_CURRENT_BINARY_DIR}/${basename}.o)
    add_custom_command(
      OUTPUT ${object_file}
      COMMAND ${CXX} -c ${CMAKE_CURRENT_SOURCE_DIR}/${source} -o ${object_file}
      DEPENDS ${source}
    )
    list(APPEND object_files ${object_file})
  else()
    set(lib_name ${directory}_${filename})
    add_library(${lib_name} ${source})
    target_link_libraries(main ${lib_name})
  endif()
endforeach()

if(object_files)
  add_library(module_objects OBJECT ${object_files})
  target_link_libraries(main module_objects)
endif()
