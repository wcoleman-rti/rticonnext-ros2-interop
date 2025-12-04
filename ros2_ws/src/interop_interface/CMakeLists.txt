cmake_minimum_required(VERSION 3.8)
project(interop_interface)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(rosidl_generator_connext_cpp REQUIRED)

# ====================================================
# Common IDL files

set(IDL_FILES
  "msg/Interop.idl"
)

# ====================================================
# Generate interfaces for both ROS2 and RTI Connext

rosidl_generate_interfaces(${PROJECT_NAME}
  ${IDL_FILES}
)

# Export dependencies
ament_export_dependencies(rosidl_default_runtime)

# Install QoS profiles
install(FILES
  config/USER_QOS_PROFILES.xml
  DESTINATION share/${PROJECT_NAME}/config
)

ament_package()
