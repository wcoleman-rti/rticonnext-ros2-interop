cmake_minimum_required(VERSION 3.8)
project(connextidl_typesupport_cpp2)

find_package(ament_cmake REQUIRED)
find_package(connextdds_cpp2 REQUIRED)

ament_package(
  CONFIG_EXTRAS "connextidl_typesupport_cpp2-extras.cmake.in"
)

install(
  DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME}
)