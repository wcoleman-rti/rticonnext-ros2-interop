
if(NOT TARGET ${rosidl_generate_interfaces_TARGET}__rosidl_generator_cpp)
  message(FATAL_ERROR
    "The 'rosidl_generator_cpp' extension must be executed before the "
    "'connextidl_typesupport_cpp2' extension.")
endif()

find_package(connextdds_cpp2 REQUIRED)
ament_export_dependencies(connextdds_cpp2)

include(ConnextDdsCodegen)

set(LANG C++11)
connextdds_sanitize_language(
  LANG ${LANG}
  VAR _lang_var
)

set(_output_path
  "${CMAKE_CURRENT_BINARY_DIR}/connextidl_typesupport_cpp2/${PROJECT_NAME}")
set(_generated_headers "")
set(_generated_sources "")
foreach(_idl_tuple ${rosidl_generate_interfaces_IDL_TUPLES})
  string(REGEX REPLACE ":([^:]*)$" ";\\1" _idl_list "${_idl_tuple}")
  list(GET _idl_list 0 _idl_abspath)
  list(GET _idl_list 1 _idl_relpath)
  file(TO_CMAKE_PATH "${_idl_abspath}" _idl_abspath)
  file(TO_CMAKE_PATH "${_idl_relpath}" _idl_relpath)
  file(TO_CMAKE_PATH "${_idl_abspath}/${_idl_relpath}" _abs_idl_file)
  get_filename_component(_idl_name ${_idl_relpath} NAME_WE)
  get_filename_component(_idl_dir ${_idl_relpath} DIRECTORY)

  connextdds_rtiddsgen_run(
    VAR "${_idl_name}"
    IDL_FILE "${_abs_idl_file}"
    OUTPUT_DIRECTORY "${_output_path}/${_idl_dir}"
    LANG ${LANG}
    DISABLE_PREPROCESSOR
  )

  list(APPEND _generated_headers
    ${${_idl_name}_${_lang_var}_HEADERS}
  )
  list(APPEND _generated_sources
    ${${_idl_name}_${_lang_var}_SOURCES}
  )
endforeach()

set(_target_suffix "__connextidl_typesupport_cpp2")

add_library(${rosidl_generate_interfaces_TARGET}${_target_suffix} 
  ${connextidl_typesupport_cpp2_LIBRARY_TYPE} 
  ${_generated_headers} ${_generated_sources}
)

if(rosidl_generate_interfaces_LIBRARY_NAME)
  set_target_properties(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PROPERTIES OUTPUT_NAME "${rosidl_generate_interfaces_LIBRARY_NAME}${_target_suffix}")
endif()
set_target_properties(${rosidl_generate_interfaces_TARGET}${_target_suffix}
  PROPERTIES
    DEFINE_SYMBOL "CONNEXTIDL_TYPESUPPORT_CPP_BUILDING_DLL"
    CXX_STANDARD 17)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(${rosidl_generate_interfaces_TARGET}${_target_suffix}
    PUBLIC
    "$<BUILD_INTERFACE:${_output_path}>"
    "$<INSTALL_INTERFACE:include>"
    "$<INSTALL_INTERFACE:include/connext>"
)

# Depend on the target created by rosidl_generator_cpp
target_link_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix} PUBLIC
  connextdds_cpp2::connextdds_cpp2)

foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  target_link_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix} PUBLIC
    ${${_pkg_name}_TARGETS${_target_suffix}})
endforeach()

# Make top level generation target depend on this library
add_dependencies(
  ${rosidl_generate_interfaces_TARGET}
  ${rosidl_generate_interfaces_TARGET}${_target_suffix}
)

if(NOT rosidl_generate_interfaces_SKIP_INSTALL)
  install(
    TARGETS ${rosidl_generate_interfaces_TARGET}${_target_suffix}
    EXPORT ${rosidl_generate_interfaces_TARGET}${_target_suffix}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )
  install(DIRECTORY ${_output_path}/
    DESTINATION include/connext/${rosidl_generate_interfaces_TARGET}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
  )

  # Export old-style CMake variables
  ament_export_libraries(${rosidl_generate_interfaces_TARGET}${_target_suffix})

  # Export modern CMake targets
  ament_export_targets(${rosidl_generate_interfaces_TARGET}${_target_suffix})
  rosidl_export_typesupport_targets(${_target_suffix}
    ${rosidl_generate_interfaces_TARGET}${_target_suffix})
endif()

if(BUILD_TESTING AND rosidl_generate_interfaces_ADD_LINTER_TESTS)
  find_package(ament_cmake_cppcheck REQUIRED)
  ament_cppcheck(
    TESTNAME "cppcheck_connextidl_typesupport_cpp2"
    "${_output_path}")

  find_package(ament_cmake_cpplint REQUIRED)
  get_filename_component(_cpplint_root "${_output_path}" DIRECTORY)
  ament_cpplint(
    TESTNAME "cpplint_connextidl_typesupport_cpp2"
    # the generated code might contain longer lines for templated types
    MAX_LINE_LENGTH 999
    ROOT "${_cpplint_root}"
    "${_output_path}")

  find_package(ament_cmake_uncrustify REQUIRED)
  ament_uncrustify(
    TESTNAME "uncrustify_connextidl_typesupport_cpp2"
    # the generated code might contain longer lines for templated types
    # a value of zero tells uncrustify to ignore line length
    MAX_LINE_LENGTH 0
    "${_output_path}")
endif()