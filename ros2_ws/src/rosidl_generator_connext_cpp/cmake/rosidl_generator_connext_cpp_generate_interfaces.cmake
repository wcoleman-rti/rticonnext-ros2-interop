# Copyright 2024 RTI
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Find RTI Connext DDS
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../resources/cmake/rticonnextdds-cmake-utils/cmake/Modules"
    "$ENV{NDDSHOME}/resource/cmake"
)
find_package(RTIConnextDDS REQUIRED)

include(ConnextDdsCodegen)

# Debug: Print what we received
message(STATUS "Connext generator called for target: ${rosidl_generate_interfaces_TARGET}")

if(NOT rosidl_generate_interfaces_TARGET)
  message(FATAL_ERROR "TARGET argument is required")
endif()

if(NOT rosidl_generate_interfaces_IDL_TUPLES)
  message(FATAL_ERROR "No IDL files provided for Connext C++ generation")
endif()

message(STATUS "IDL files: ${rosidl_generate_interfaces_ABS_IDL_FILES}")

set(connext_cpp_typesupport_sources)

# Process each IDL file
foreach(_idl_tuple ${rosidl_generate_interfaces_IDL_TUPLES})

  string(REGEX REPLACE ":([^:]*)$" ";\\1" _idl_list "${_idl_tuple}")
  list(GET _idl_list 0 _idl_abspath)
  list(GET _idl_list 1 _idl_relpath)
  file(TO_CMAKE_PATH "${_idl_abspath}" _idl_abspath)
  file(TO_CMAKE_PATH "${_idl_relpath}" _idl_relpath)
  get_filename_component(_idl_name ${_idl_relpath} NAME_WE)
  get_filename_component(_idl_dir ${_idl_relpath} DIRECTORY)

  message(STATUS "Generating Connext C++ typesupport for IDL: ${_idl_relpath}")
  message(STATUS "  Absolute path: ${_idl_abspath}")

  # Generate Connext C++ code
  connextdds_rtiddsgen_run(
    VAR "${_idl_name}"
    IDL_FILE "${_idl_abspath}/${_idl_relpath}"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_connext_cpp/${_idl_dir}"
    LANG C++11
    EXTRA_ARGS
      -ppOption -P
  )

  list(APPEND connext_cpp_typesupport_sources
    ${${_idl_name}_CXX11_GENERATED_SOURCES}
  )
endforeach()

# Create the RTI Connext typesupport library
if(connext_cpp_typesupport_sources)
  set(connext_target "${rosidl_generate_interfaces_TARGET}__connext_typesupport_cpp")

  add_library(${connext_target} SHARED ${connext_cpp_typesupport_sources})

  target_link_libraries(${connext_target}
    PUBLIC
    RTIConnextDDS::cpp2_api
    RTIConnextDDS::metp  # for Zero Copy
  )

  # Make generated headers available
  target_include_directories(${connext_target}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/rti>
    $<INSTALL_INTERFACE:include/${rosidl_generate_interfaces_TARGET}/rti>
  )

  set_target_properties(${connext_target}
    PROPERTIES
    EXPORT_NAME connext_typesupport_cpp
    OUTPUT_NAME "${connext_target}"
  )

  # Install the library
  install(TARGETS ${connext_target}
    EXPORT export_${connext_target}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
  )

  # Install generated headers
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_connext_cpp/
    DESTINATION include/${rosidl_generate_interfaces_TARGET}/rti/${rosidl_generate_interfaces_TARGET}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
  )

  # Export the target
  install(EXPORT export_${connext_target}
    FILE ${connext_target}.cmake
    NAMESPACE ${rosidl_generate_interfaces_TARGET}::
    DESTINATION lib/cmake/${rosidl_generate_interfaces_TARGET}
  )

  ament_export_targets(export_${connext_target} HAS_LIBRARY_TARGET)

  message(STATUS "Generated Connext typesupport library: ${connext_target}")
else()
  message(WARNING "No Connext C++ typesupport sources were generated.")
endif()



rosidl_get_typesupport_target(rosidl_target
  ${rosidl_generate_interfaces_TARGET} rosidl_typesupport_cpp
)
if(TARGET ${rosidl_target})
  set_target_properties(${rosidl_target}
    PROPERTIES
      EXPORT_NAME rosidl_typesupport_cpp
      OUTPUT_NAME "${rosidl_target}"
  )

  # Install the library
  install(TARGETS ${rosidl_target}
    EXPORT export_${rosidl_target}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
  )

  # Export the target
  message(STATUS "Exporting ROSIDL typesupport cpp target: ${rosidl_target}")
  install(EXPORT export_${rosidl_target}
    FILE ${rosidl_target}.cmake
    NAMESPACE ${rosidl_generate_interfaces_TARGET}::
    DESTINATION lib/cmake/${rosidl_generate_interfaces_TARGET}
  )

  ament_export_targets(export_${rosidl_target} HAS_LIBRARY_TARGET)

  message(STATUS "Installed ROSIDL typesupport cpp target: ${rosidl_target}")
else()
  message(WARNING "ROSIDL typesupport cpp target not found: ${rosidl_target}")
endif()
