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


# set(_idl_mod_dir "${CMAKE_CURRENT_BINARY_DIR}/share")
set(_idl_mod_dir "${CMAKE_CURRENT_BINARY_DIR}/connextidl_typesupport/share")

macro(sanitize_idl_file INPUT_IDL OUTPUT_IDL_VAR)
  if("${RTICONNEXTDDS_VERSION}" VERSION_GREATER_EQUAL "7.2.0" AND 
    "${RTICONNEXTDDS_VERSION}" VERSION_LESS "7.5.0")

    # Use a modified version of the IDL file in the build dir to avoid expected syntax errors
    # Extract relative path components to recreate directory structure
    get_filename_component(_idl_name "${INPUT_IDL}" NAME)
    get_filename_component(_idl_parent_dir "${INPUT_IDL}" DIRECTORY)
    get_filename_component(_idl_parent_type "${_idl_parent_dir}" NAME)
    get_filename_component(_idl_package_dir "${_idl_parent_dir}" DIRECTORY)
    get_filename_component(_idl_package_name "${_idl_package_dir}" NAME)
    set(_idl_relpath "${_idl_package_name}/${_idl_parent_type}/${_idl_name}")

    # Create output path
    set(${OUTPUT_IDL_VAR} "${_idl_mod_dir}/${_idl_relpath}")
    get_filename_component(_idl_file_dir "${${OUTPUT_IDL_VAR}}" DIRECTORY)
    file(MAKE_DIRECTORY "${_idl_file_dir}")

    message(STATUS "  Applying CODEGENII-2200 workaround for RTI Connext DDS version ${RTICONNEXTDDS_VERSION}")
    message(STATUS "    Input:  ${INPUT_IDL}")
    message(STATUS "    Output: ${${OUTPUT_IDL_VAR}}")

    # CODEGENII-2200 - @verbatim annotation handled incorrectly, results in error
    # Workaround: Use Python script to strip out the @verbatim annotations
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    set(_strip_script "${connextidl_typesupport_cpp2_DIR}/strip_verbatim.py")

    execute_process(
      COMMAND ${Python3_EXECUTABLE} "${_strip_script}" "${INPUT_IDL}" "${${OUTPUT_IDL_VAR}}"
      RESULT_VARIABLE _strip_result
      ERROR_VARIABLE _strip_error
    )

    if(NOT _strip_result EQUAL 0)
      message(FATAL_ERROR "Failed to strip @verbatim annotations: ${_strip_error}")
    endif()

  else()
    # Use the original IDL file directly
    set(${OUTPUT_IDL_VAR} "${INPUT_IDL}")
  endif()
endmacro()

macro(get_idl_include_dir IDL_FILE OUTPUT_DIR_VAR)
  get_filename_component(_idl_include_dir "${IDL_FILE}" DIRECTORY)
  get_filename_component(_idl_include_dir "${_idl_include_dir}" DIRECTORY)
  get_filename_component(_idl_include_dir "${_idl_include_dir}" DIRECTORY)
  set(${OUTPUT_DIR_VAR} "${_idl_include_dir}")
endmacro()


set(_dependency_files "")
set(_dependencies_include_dirs "")
set(_dependencies "")
foreach(_pkg_name ${rosidl_generate_interfaces_DEPENDENCY_PACKAGE_NAMES})
  foreach(_idl_file ${${_pkg_name}_IDL_FILES})
    set(_abs_idl_file "${${_pkg_name}_DIR}/../${_idl_file}")
    normalize_path(_abs_idl_file "${_abs_idl_file}")
    
    # Sanitize the dependency IDL file if necessary
    sanitize_idl_file("${_abs_idl_file}" _sanitized_idl_file)
    
    # Determine include directory for this dependency
    get_idl_include_dir("${_sanitized_idl_file}" _sanitized_idl_include_dir)
    message(STATUS "Dependency package '${_pkg_name}' IDL include dir: ${_sanitized_idl_include_dir}")
    list(APPEND _dependencies_include_dirs "${_sanitized_idl_include_dir}")

    list(APPEND _dependency_files "${_sanitized_idl_file}")
    list(APPEND _dependencies "${_pkg_name}:${_abs_idl_file}")
  endforeach()
endforeach()
list(APPEND _dependencies_include_dirs "${_idl_mod_dir}")
list(REMOVE_DUPLICATES _dependencies_include_dirs)

set(target_dependencies
  ${rosidl_generate_interfaces_ABS_IDL_FILES}
  ${_dependency_files})
foreach(dep ${target_dependencies})
  if(NOT EXISTS "${dep}")
    message(FATAL_ERROR "Target dependency '${dep}' does not exist")
  endif()
endforeach()

set(_output_path
  "${CMAKE_CURRENT_BINARY_DIR}/connextidl_typesupport_cpp2/${PROJECT_NAME}")
set(_generated_headers "")
set(_generated_sources "")


set(_rtiddsgen_extra_args "")
if(RTICONNEXTDDS_VERSION VERSION_GREATER_EQUAL "7.2.0")
  list(APPEND _rtiddsgen_extra_args "-standard" "IDL4_CPP")
endif()

foreach(_idl_tuple ${rosidl_generate_interfaces_IDL_TUPLES})
  string(REGEX REPLACE ":([^:]*)$" ";\\1" _idl_list "${_idl_tuple}")
  list(GET _idl_list 0 _idl_abspath)
  list(GET _idl_list 1 _idl_relpath)
  file(TO_CMAKE_PATH "${_idl_abspath}" _idl_abspath)
  file(TO_CMAKE_PATH "${_idl_relpath}" _idl_relpath)
  file(TO_CMAKE_PATH "${_idl_abspath}/${_idl_relpath}" _abs_idl_file)
  get_filename_component(_idl_name ${_idl_relpath} NAME_WE)
  get_filename_component(_idl_dir ${_idl_relpath} DIRECTORY)

  message(STATUS "======================================")
  message(STATUS "Generating Connext typesupport for: ${_abs_idl_file}")
  message(STATUS "  IDL name: ${_idl_name}")
  message(STATUS "  Dependencies include dirs: ${_dependencies_include_dirs}")
  
  # Apply any necessary sanitization to the IDL file
  # (e.g., stripping @verbatim annotations for certain RTI versions)
  # Note: This creates a modified copy of the IDL file in the build directory
  #  to avoid altering the original source file.
  sanitize_idl_file("${_abs_idl_file}" _idl_file)

  # Determine include directory for this IDL file
  get_idl_include_dir("${_idl_file}" _idl_include_dir)
  message(STATUS "  IDL include dir: ${_idl_include_dir}")


  connextdds_rtiddsgen_run(
    VAR "${_idl_name}"
    IDL_FILE "${_idl_file}"
    OUTPUT_DIRECTORY "${_output_path}/${_idl_dir}"
    LANG ${LANG}
    # DISABLE_PREPROCESSOR
    # DEPENDS ${target_dependencies}
    INCLUDE_DIRS ${_dependencies_include_dirs} ${_idl_include_dir}
    EXTRA_ARGS ${_rtiddsgen_extra_args}
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
    "$<BUILD_INTERFACE:${_output_path}/..>"
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