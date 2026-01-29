cmake_minimum_required(VERSION 3.8)
project(connextidl_typesupport_cpp2)

find_package(ament_cmake REQUIRED)
find_package(connextdds_cpp2 REQUIRED)

# ============================================
# Install core package files
# ============================================
ament_package(
  CONFIG_EXTRAS 
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/connextidl_typesupport_cpp2-extras.cmake.in"
)

install(
  DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME}
)