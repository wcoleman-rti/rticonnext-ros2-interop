cmake_minimum_required(VERSION 3.20)
project(interop)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# Generate message
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Interop.idl"
)

ament_export_dependencies(rosidl_default_runtime)

rosidl_get_typesupport_target(cpp_typesupport_target
  ${PROJECT_NAME} rosidl_typesupport_cpp)

# Prepare executables
add_executable(interop_publisher src/interop_publisher.cpp)
add_executable(interop_subscriber src/interop_subscriber.cpp)

# Include headers
target_include_directories(interop_publisher 
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)
target_include_directories(interop_subscriber
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

# Link against the generated typesupport library
target_link_libraries(interop_publisher
    "${cpp_typesupport_target}" 
    rclcpp::rclcpp)
target_link_libraries(interop_subscriber
   "${cpp_typesupport_target}" 
   rclcpp::rclcpp)

# Require C99 and C++17
target_compile_features(interop_publisher PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
target_compile_features(interop_subscriber PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17

# Install executables
install(TARGETS 
  interop_publisher
  interop_subscriber
  DESTINATION lib/${PROJECT_NAME})

ament_package()
