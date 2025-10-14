
cmake_minimum_required(VERSION 3.11)
project(rti-connext-example-cpp11)

# Add the folder where the FindRTIConnextDDS.cmake file lives
list(APPEND CMAKE_MODULE_PATH
    "$ENV{NDDSHOME}/resource/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../resources/cmake"
)

# Find the RTI Connext libraries
find_package(RTIConnextDDS
    "7.3.0"
    REQUIRED
    COMPONENTS
        core
)

include(ConnextDdsCodegen)

if (DEFINED USE_SHMEM_REF)
    add_definitions(-DUSE_SHMEM_REF=${USE_SHMEM_REF})
    set(EXTRA_RTIDDSGEN_ARGS "-DUSE_SHMEM_REF=${USE_SHMEM_REF}")
    set(EXTRA_RTICONNEXT_LIBS RTIConnextDDS::metp)
endif()

# Generate the sources for the types
connextdds_rtiddsgen_run(
    IDL_FILE
        "${CMAKE_CURRENT_SOURCE_DIR}/msg/Instance.idl"
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/msg"
    LANG C++11
    EXTRA_ARGS
        ${EXTRA_RTIDDSGEN_ARGS}
)

# -----------------------------------------------
# Collect the generated sources

# Create an object library for the generated sources
add_library(
    Instance_types 
    OBJECT
        "${Instance_CXX11_GENERATED_SOURCES}"
)

target_link_libraries(
    Instance_types
    PUBLIC
        RTIConnextDDS::cpp2_api
        ${EXTRA_RTICONNEXT_LIBS}
)

target_include_directories(
    Instance_types
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>"
)

# ------------------------------------------------
# Define the executables

# Create the executables
add_executable(
    Instance_publisher
    "${CMAKE_CURRENT_SOURCE_DIR}/Instance_publisher.cxx"
)

add_executable(
    Instance_subscriber
    "${CMAKE_CURRENT_SOURCE_DIR}/Instance_subscriber.cxx"
)

# Link against the RTI Connext libraries
target_link_libraries(
    Instance_publisher
    PRIVATE
        Instance_types
)

target_link_libraries(
    Instance_subscriber
    PRIVATE
        Instance_types
)

# ---------------------------------------------------
# Install the executables

install(
    TARGETS
        Instance_publisher
        Instance_subscriber
    RUNTIME DESTINATION bin
)

set(QOS_XML_FILE "${CMAKE_CURRENT_SOURCE_DIR}/USER_QOS_PROFILES.xml")
if(DEFINED QOS_XML_FILE AND EXISTS "${QOS_XML_FILE}")
    install(
        FILES
            "${QOS_XML_FILE}"
        DESTINATION bin
    )
endif()