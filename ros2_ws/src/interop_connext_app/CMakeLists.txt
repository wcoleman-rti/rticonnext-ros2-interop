cmake_minimum_required(VERSION 3.8)
project(interop_connext_app)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Add the folder where the FindRTIConnextDDS.cmake file lives
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../resources/cmake/rticonnextdds-cmake-utils/cmake/Modules"
    "$ENV{NDDSHOME}/resource/cmake"
)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(RTIConnextDDS "6.0.1" REQUIRED COMPONENTS core)
find_package(interop_interface REQUIRED)

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
    interop_interface::connext_typesupport_cpp
    RTIConnextDDS::cpp2_api)
target_link_libraries(interop_subscriber
    interop_interface::connext_typesupport_cpp
    RTIConnextDDS::cpp2_api)

# Require C99 and C++17
target_compile_features(interop_publisher PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
target_compile_features(interop_subscriber PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17

# Install executables
install(TARGETS 
  interop_publisher
  interop_subscriber
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
