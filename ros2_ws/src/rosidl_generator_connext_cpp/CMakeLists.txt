cmake_minimum_required(VERSION 3.8)
project(rosidl_generator_connext_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Add the folder where the FindRTIConnextDDS.cmake file lives
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../resources/cmake/rticonnextdds-cmake-utils/cmake/Modules"
    "$ENV{NDDSHOME}/resource/cmake"
)

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rosidl_generator_c REQUIRED)
find_package(rosidl_generator_cpp REQUIRED)
find_package(rosidl_typesupport_interface REQUIRED)
find_package(RTIConnextDDS "6.0.1" REQUIRED)

# Install Python modules
ament_python_install_package(${PROJECT_NAME})

# Install the generator plugin script
install(
  PROGRAMS rosidl_generator_connext_cpp/__init__.py
  DESTINATION lib/${PROJECT_NAME}
)

# Install cmake files
install(
  FILES
    cmake/rosidl_generator_connext_cpp_generate_interfaces.cmake
    cmake/rosidl_generator_connext_cpp-extras.cmake
  DESTINATION share/${PROJECT_NAME}/cmake
)

# Install resource files
install(
  DIRECTORY resource/
  DESTINATION share/${PROJECT_NAME}/resource
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package(
  CONFIG_EXTRAS "cmake/rosidl_generator_connext_cpp-extras.cmake"
)