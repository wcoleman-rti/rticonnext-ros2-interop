# ConnextIDL TypeSupport C++2

This package provides Connext DDS C++2 typesupport generation for ROS2 message types.

## Features

- Core macro `connextidl_generate_interfaces_for_ros2_pkg()` for generating Connext typesupport
- Automatic reflection package generation for ROS2 distro packages
- Auto-detection of available ROS2 packages with IDL files

## Usage

### Option 1: Manual Reflection Package

Create your own package and use the macro:

```cmake
cmake_minimum_required(VERSION 3.8)
project(connextidl_my_msgs)

find_package(ament_cmake REQUIRED)
find_package(connextidl_typesupport_cpp2 REQUIRED)

connextidl_generate_interfaces_for_ros2_pkg("my_msgs")

ament_package()
```

### Option 2: Auto-Generate Reflection Packages

Generate reflection packages automatically for ROS2 distro packages:

#### Auto-detect all ROS2 packages with IDL files:
```bash
cd ros2_ws
colcon build --packages-select connextidl_typesupport_cpp2 \
  --cmake-args -DCONNEXTIDL_TYPESUPPORT_CPP2_GENERATE_REFLECTION_PACKAGES=ON
```

#### Specify specific packages:
```bash
colcon build --packages-select connextidl_typesupport_cpp2 \
  --cmake-args -DCONNEXTIDL_TYPESUPPORT_CPP2_GENERATE_REFLECTION_PACKAGES=ON \
               -DCONNEXTIDL_TYPESUPPORT_CPP2_ROS2_PACKAGES="std_msgs;sensor_msgs;geometry_msgs;nav_msgs"
```

#### Custom output directory:
```bash
colcon build --packages-select connextidl_typesupport_cpp2 \
  --cmake-args -DCONNEXTIDL_TYPESUPPORT_CPP2_GENERATE_REFLECTION_PACKAGES=ON \
               -DCONNEXTIDL_TYPESUPPORT_CPP2_REFLECTION_OUTPUT_DIR="/path/to/output"
```

### Building Generated Packages

After generating reflection packages, build them:

```bash
# Build specific packages
colcon build --packages-select connextidl_std_msgs connextidl_sensor_msgs

# Or build all connextidl packages
colcon build --packages-select connextidl_*
```

## CMake Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `CONNEXTIDL_TYPESUPPORT_CPP2_GENERATE_REFLECTION_PACKAGES` | OPTION | OFF | Enable automatic reflection package generation |
| `CONNEXTIDL_TYPESUPPORT_CPP2_ROS2_PACKAGES` | STRING | "" | Semicolon-separated list of ROS2 packages (empty = auto-detect) |
| `CONNEXTIDL_TYPESUPPORT_CPP2_REFLECTION_OUTPUT_DIR` | PATH | `../` | Output directory for generated packages |

## Example: Full Workflow

```bash
# 1. Generate reflection packages for common ROS2 message types
cd ~/ros2_ws
colcon build --packages-select connextidl_typesupport_cpp2 \
  --cmake-args -DCONNEXTIDL_TYPESUPPORT_CPP2_GENERATE_REFLECTION_PACKAGES=ON \
               -DCONNEXTIDL_TYPESUPPORT_CPP2_ROS2_PACKAGES="std_msgs;sensor_msgs;geometry_msgs;nav_msgs"

# 2. Build the generated packages
colcon build --packages-select connextidl_std_msgs connextidl_sensor_msgs \
                               connextidl_geometry_msgs connextidl_nav_msgs

# 3. Use in your application
# In your package's CMakeLists.txt:
# find_package(connextidl_std_msgs REQUIRED)
```

## Creating a Metapackage

For convenience, you can create a metapackage that depends on all generated packages:

```xml
<!-- connextidl_common_interfaces/package.xml -->
<?xml version="1.0"?>
<package format="3">
  <name>connextidl_common_interfaces</name>
  <version>0.0.1</version>
  <description>Metapackage for common ConnextIDL message types</description>
  
  <exec_depend>connextidl_std_msgs</exec_depend>
  <exec_depend>connextidl_sensor_msgs</exec_depend>
  <exec_depend>connextidl_geometry_msgs</exec_depend>
  <!-- Add more as needed -->
  
  <export>
    <build_type>ament_cmake</build_type>
  </export>
</package>
```

## Architecture

This package uses a "reflection package" approach where:
- **connextidl_typesupport_cpp2** is the core package providing the generation macro
- **connextidl_<ros2_pkg>** packages are lightweight reflection packages that invoke the macro for specific ROS2 distro packages
- Each reflection package mirrors the structure of its corresponding ROS2 package

This design allows for:
- ✅ Granular dependencies (only depend on what you need)
- ✅ Faster builds (only rebuild affected packages)
- ✅ Easy extension to other language bindings (e.g., connextidl_typesupport_c, connextidl_typesupport_py)

## License

Apache-2.0
