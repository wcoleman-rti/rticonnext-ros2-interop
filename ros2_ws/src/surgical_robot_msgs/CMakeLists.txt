cmake_minimum_required(VERSION 3.8)
project(surgical_robot_msgs)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(connextidl_typesupport_cpp2 REQUIRED)  # Native Connext "modern" C++ code generation
find_package(std_msgs REQUIRED)

# Generate ROS2 interfaces
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/SurgeryCommand.msg"
  "msg/SurgeryState.msg"
  DEPENDENCIES std_msgs
)

install(FILES
  config/USER_QOS_PROFILES.xml
  DESTINATION share/${PROJECT_NAME}/config
)

ament_package()
