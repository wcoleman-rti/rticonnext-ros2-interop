
cmake_minimum_required(VERSION 3.11)
project(rti-connext-example)

# Use default install prefix of <source_dir>
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}" CACHE PATH "Install prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/instances)