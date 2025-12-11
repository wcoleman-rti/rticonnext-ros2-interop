cmake_minimum_required(VERSION 3.8)
project(interop_interface)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# list(APPEND CMAKE_MODULE_PATH
#     "${CMAKE_CURRENT_SOURCE_DIR}/../../../resources/cmake/rticonnextdds-cmake-utils/cmake/Modules"
#     "$ENV{NDDSHOME}/resource/cmake"
# )

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(connextidl_typesupport_cpp2 REQUIRED)  # Native Connext "modern" C++ code generation

# ====================================================
# Generate interfaces

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Interop.idl"
  "msg/Status.idl"
)

# Export dependencies
ament_export_dependencies(rosidl_default_runtime)
ament_export_dependencies(connextidl_typesupport_cpp2)

# Install QoS profiles
install(FILES
  config/USER_QOS_PROFILES.xml
  DESTINATION share/${PROJECT_NAME}/config
)

ament_package()
