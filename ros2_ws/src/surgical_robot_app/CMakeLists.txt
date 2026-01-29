cmake_minimum_required(VERSION 3.8)
project(surgical_robot_app)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(surgical_robot_msgs REQUIRED)

# Prepare executables
add_executable(surgical_robot_publisher src/surgical_robot_publisher.cpp)
add_executable(surgical_robot_subscriber src/surgical_robot_subscriber.cpp)

# Include headers
# target_include_directories(surgical_robot_publisher
#   PUBLIC
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
# target_include_directories(surgical_robot_subscriber
#   PUBLIC
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

# Link against the generated typesupport library
# Use ament_target_dependencies to link at the package level
ament_target_dependencies(surgical_robot_publisher
  surgical_robot_msgs
  rclcpp)
ament_target_dependencies(surgical_robot_subscriber
  surgical_robot_msgs
  rclcpp)
# Alternatively, link against the explicit typesupport library
# target_link_libraries(surgical_robot_publisher
#     surgical_robot_msgs::surgical_robot_msgs__rosidl_typesupport_cpp
#     surgical_robot_msgs::surgical_robot_msgs__connextidl_typesupport_cpp2
#     rclcpp::rclcpp)
# target_link_libraries(surgical_robot_subscriber
#     surgical_robot_msgs::surgical_robot_msgs__rosidl_typesupport_cpp
#     surgical_robot_msgs::surgical_robot_msgs__connextidl_typesupport_cpp2
#     rclcpp::rclcpp)

# Require C99 and C++17
target_compile_features(surgical_robot_publisher PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
target_compile_features(surgical_robot_subscriber PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17

# Install executables
install(TARGETS 
  surgical_robot_publisher
  surgical_robot_subscriber
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()

