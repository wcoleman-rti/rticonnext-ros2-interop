cmake_minimum_required(VERSION 3.8)
project(connextdds_cpp2)

find_package(ament_cmake REQUIRED)

set(RTICONNEXT_CMAKE_UTILS_GIT_TAG "main" CACHE STRING "Git tag, branch, or commit hash for rticonnextdds-cmake-utils")

# Fetch rticonnextdds-cmake-utils
include(FetchContent)
FetchContent_Declare(
  rticonnextdds-cmake-utils
  GIT_REPOSITORY https://github.com/rticommunity/rticonnextdds-cmake-utils.git
  GIT_TAG ${RTICONNEXT_CMAKE_UTILS_GIT_TAG}
  GIT_SHALLOW TRUE
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/rticonnextdds-cmake-utils"
)
FetchContent_MakeAvailable(rticonnextdds-cmake-utils)
FetchContent_GetProperties(rticonnextdds-cmake-utils SOURCE_DIR RTI_CMAKE_UTILS_DIR)
list(APPEND CMAKE_MODULE_PATH 
  "${RTI_CMAKE_UTILS_DIR}/cmake/Modules"
)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(RTIConnextDDS REQUIRED)

# Define interface library
add_library(${PROJECT_NAME} INTERFACE)

# Link against Connext Modern C++ API
target_link_libraries(
  ${PROJECT_NAME}
  INTERFACE 
  RTIConnextDDS::cpp2_api)

# Install interface library
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}
)

# Install export targets
install(
  EXPORT ${PROJECT_NAME}
  FILE ${PROJECT_NAME}.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION share/${PROJECT_NAME}/cmake
)

# Export target
ament_export_targets(${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(RTIConnextDDS)

# Export config files
ament_package(
  CONFIG_EXTRAS 
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/connextdds_cpp2-extras.cmake.in"
)

# Install cmake utils and configure extras to set CMAKE_MODULE_PATH
install(
  DIRECTORY ${RTI_CMAKE_UTILS_DIR}
  DESTINATION share/${PROJECT_NAME}/cmake/
  PATTERN "cmake/Modules/*"
)
