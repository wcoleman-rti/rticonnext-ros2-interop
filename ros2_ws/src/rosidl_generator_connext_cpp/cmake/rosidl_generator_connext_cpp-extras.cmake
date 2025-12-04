# Add this package to the list of available generators
set(rosidl_generator_connext_cpp_FOUND TRUE)

# Register this generator with the rosidl system
ament_register_extension(
  "rosidl_generate_idl_interfaces"
  "rosidl_generator_connext_cpp"
  "rosidl_generator_connext_cpp_generate_interfaces.cmake"
)