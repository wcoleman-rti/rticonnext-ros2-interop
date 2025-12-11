# Native Connext - ROS2 rmw_connextdds Interop

## Overview

This set of examples demonstrates interoperability between ROS2 Humble pub/sub apps using:

1. rmw_connextdds
2. native RTI Connext 7.3.0
3. mixed rmw_connextdds and native RTI Connext 7.3.0

## Requirements

1. [ROS2 Humble](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debs.html#install-ros-2)
    * Using [RTI Connext 7.3.0](https://docs.ros.org/en/humble/Installation/RMW-Implementations/DDS-Implementations/Working-with-RTI-Connext-DDS.html#rti-connext-dds) will require [building rmw_connextdds from source](https://docs.ros.org/en/humble/Installation/RMW-Implementations/DDS-Implementations/Working-with-RTI-Connext-DDS.html#building-rmw-connextdds-from-source-code).
2. [RTI Connext 7.3.0](https://community.rti.com/static/documentation/connext-dds/current/doc/manuals/debian_packages/install.html)

## Build

*Note: rmw_connextdds should be built against the same version of RTI Connext you plan to build the native Connext components against. You may need to [build rmw_connextdds from source](https://docs.ros.org/en/humble/Installation/RMW-Implementations/DDS-Implementations/Working-with-RTI-Connext-DDS.html#building-rmw-connextdds-from-source-code).*

```sh
source /opt/ros/humble/setup.bash
source /opt/rti.com/rti_connext_dds-7.3.0/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.bash
cd ros2_ws
colcon build --symlink-install
```

## Run

Common environment:

```sh
# Setup the ROS2 environment
cd ros2_ws
source /opt/ros/humble/setup.bash
source /opt/rti.com/rti_connext_dds-7.3.0/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.bash
source <path/to/rmw_connextdds>/install.setup.bash  # if rmw_connextdds was built from source as a ROS2 package
source install/setup.bash

# Use rmw_connextdds and point to QoS file
export RMW_IMPLEMENTATION=rmw_connextdds
export NDDS_QOS_PROFILES=install/interop_interface/share/interop_interface/config/USER_QOS_PROFILES.xml
```

Publisher:

```sh
ros2 run <app_type> interop_publisher 3  # <3> is used as the key 'id' field value of data published
# <app_type> can be:
#   - interop_rmw_app
#   - interop_connext_app
#   - interop_mixed_app
```

Subscriber:

```sh
ros2 run <app_type> interop_subscriber
# <app_type> can be:
#   - interop_rmw_app
#   - interop_connext_app
#   - interop_mixed_app
```

## Package Descriptions

### connextdds_cpp2

This package:

1. Exports the RTIConnextDDS::cpp2_api library (+dependencies) as an ament package.
2. Installs the rticonnextdds-cmake-utils and loads the path to `CMAKE_MODULE_PATH`.

This package does not build anything. It simply makes the RTI Connext Modern C++ libraries and dependencies available for downstream ament cmake packages.

Before building this package, the RTI environment should be set to properly find Connext libraries:

```sh
source <path/to/rti_connext_dds-#-#-#>/resource/scripts/rtisetenv_<platform>.bash
```

### connextidl_typesupport_cpp2

This package implements an extension to the `rosidl_generate_idl_interfaces` extension point to generate code for RTI Connext Modern C++ api.

By finding this package with `find_package(connextidl_typesupport_cpp2)`, a single call to `rosidl_generate_interfaces()`
will generate code for both default rosidl generators as well as RTI Code Generator.

This package does not build anything. It simply provides a hook to execute RTI Code Generator, and export a library for a set of IDL files passed to `rosidl_generate_interfaces()`.

Usage:

```cmake
find_package(rosidl_default_generators REQUIRED)
find_package(connextidl_typesupport_cpp2 REQUIRED)  # Native Connext "modern" C++ code generation

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Interop.idl"
  "msg/Status.idl"
)
```

Note: installed Connext headers can be found at either `<package_name>/msg/Foo.hpp` or `connext/<package_name>/msg/Foo.hpp` to avoid header include clashes.

To link against only the Connext-generated libraries:

```cmake
target_link_libraries(myTarget
    interface_pkg::interface_pkg__connextidl_typesupport_cpp2)
```

Default rosidl-generated libraries are also available to link against individually, for example `rosidl_typesupport_cpp`:

```cmake
target_link_libraries(myTarget
    interface_pkg::interface_pkg__rosidl_typesupport_cpp)
```

### interop_interface

This package demonstrates generating code for both default rosidl generators as well as `connextidl_typesupport_cpp2` for a set of IDL files.

### interop_rmw_app

This package demonstrates building a ROS2 application against libraries from the example `interop_interface` package.

The publisher and subscriber applications use ROS2 RMW nodes/publishers/subscriptions only.

This package only makes use of the default rosidl-generated code/libraries.

### interop_connext_app

This package demonstrates building a ROS2 application against libraries from the example `interop_interface` package.

The publisher and subscriber applications use native Connext participants/datawriters/datareaders only.

This package only makes use of the Connext-generated code/libraries.

### interop_mixed_app

This package demonstrates building a ROS2 application against libraries from the example `interop_interface` package.

The publisher and subscriber applications use a mix of ROS2 RMW nodes/publishers/subscriptions and native Connext participants/datawriters/datareaders only.

This package makes use of both the default rosidl-generated and Connext-generated code/libraries.
