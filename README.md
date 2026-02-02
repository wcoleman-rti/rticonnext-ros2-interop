# Native Connext - ROS2 rmw_connextdds Interop

## Overview

This set of examples demonstrates interoperability between ROS2 Humble pub/sub apps using:

1. rmw_connextdds
2. native RTI Connext 7.3.0
3. mixed rmw_connextdds and native RTI Connext 7.3.0

## Requirements

1. [ROS2 Humble](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debs.html#install-ros-2)
    * Using [RTI Connext 7.3.0](https://docs.ros.org/en/humble/Installation/RMW-Implementations/DDS-Implementations/Working-with-RTI-Connext-DDS.html#rti-connext-dds) will require [building rmw_connextdds from source](https://docs.ros.org/en/humble/Installation/RMW-Implementations/DDS-Implementations/Working-with-RTI-Connext-DDS.html#building-rmw-connextdds-from-source-code).
2. [RTI Connext 7.3.0](https://community.rti.com/static/documentation/connext-dds/7.3.0/doc/manuals/debian_packages/install.html)

## Build

*Note: rmw_connextdds should be built against the same version of RTI Connext you plan to build the native Connext components against. You may need to [build rmw_connextdds from source](https://docs.ros.org/en/humble/Installation/RMW-Implementations/DDS-Implementations/Working-with-RTI-Connext-DDS.html#building-rmw-connextdds-from-source-code).*

```sh
# Add/update submodules (for: builtin_interfaces, std_msgs)
git submodule update --init --recursive

# Setup the ROS2 environment
source /opt/ros/humble/setup.bash
source /opt/rti.com/rti_connext_dds-7.3.0/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.bash

# Build package and dependencies
cd ros2_ws
colcon build --symlink-install --packages-up-to surgical_robot_app --allow-overriding builtin_interfaces std_msgs
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
export NDDS_QOS_PROFILES=install/surgical_robot_msgs/share/surgical_robot_msgs/config/USER_QOS_PROFILES.xml
```

Publisher:

```sh
ros2 run surgical_robot_app surgical_robot_publisher 3  # <3> is used as the key 'id' field value of data published
```

The following are commands that can be administered from the publisher:

* `start`   : Publishes a DOING_SURGERY command
* `stop`    : Publishes a NOT_DOING_SURGERY command
* \<other> : Publishes a command that should not change the surgery state

Subscriber:

```sh
ros2 run surgical_robot_app surgical_robot_subscriber
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

This package mirrors the implementation of [rosidl_typesupport_cpp](https://github.com/ros2/rosidl_typesupport/tree/rolling/rosidl_typesupport_cpp).

By finding this package with `find_package(connextidl_typesupport_cpp2)`, a single call to `rosidl_generate_interfaces()`
will generate code for both default rosidl generators as well as RTI Code Generator.

This package does not build anything. It simply provides a hook to execute RTI Code Generator, and export a library for a set of IDL files passed to `rosidl_generate_interface`.

Usage:

```cmake
find_package(rosidl_default_generators REQUIRED)
find_package(connextidl_typesupport_cpp2 REQUIRED)  # Native Connext "modern" C++ code generation
find_package(std_msgs REQUIRED)  # for std_msgs/Header

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/SurgeryCommand.idl"
  "msg/SurgeryState.idl"
  DEPENDENCIES std_msgs
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

### surgical_robot_msgs

This package demonstrates generating code for both default rosidl generators as well as `connextidl_typesupport_cpp2` for a set of IDL files.

It demonstrates type composition for which msg members may be composed of types defined from other packages (e.g. std_msgs/Header).

### surgical_robot_app

This package demonstrates building a ROS2 application against libraries from the example `surgical_robot_msgs` package.

The publisher and subscriber applications use a mix of ROS2 RMW nodes/publishers/subscriptions and native Connext participants/datawriters/datareaders only.

This package makes use of both the default rosidl-generated and Connext-generated code/libraries.

## Notes

### ROS2 Distro Msgs

Since *connextidl_typesupport_cpp2* is a `rosidl_generate_idl_interfaces` extension point, it means that packages available as part of the ROS2 distro/installation (e.g. *std_msgs*), have not had connextidl typesupport code generated and available as a dependency. To use a ROS2 distro package as a dependency, you should perform a source-overlay build - that is, rebuild the package, but this time with the extension point registered.

In this repository, `SurgeryCommand` and `SurgeryState` both make use of `std_msgs/Header` from the *std_msgs* ROS2 distro package. Therefore to use this member type with connextidl typesupport, *std_msgs* and all of its dependencies must be rebuilt as overlay packages with *connextidl_typesupport_cpp2* registered. This is done by pulling in the ROS2 *[rcl_interfaces](https://github.com/ros2/rcl_interfaces.git)* and *[common_interfaces](https://github.com/ros2/common_interfaces.git)* repositories as git submodules.

When building with colcon, you can acknowledge and suppress warnings for building overlay packages by using the `--allow-overriding` argument.

```shell
colcon build <...> --allow-overriding builtin_interfaces std_msgs
```

## Known Issues

### ROS2 Message Definition Comment Content

`rosidl_generate_interfaces()` converts .msg content into .idl. *rtiddsgen* generates typesupport code from .idl. `rosidl_generate_interfaces()` translates comment content to the @verbatim .idl annotation.

From RTI Connext 7.2.0 to 7.4.0, *rtiddsgen* fails when processing @verbatim annotations, as opposed to ignoring them.

As a workaround, a [strip_verbatim.py](./ros2_ws/src/connextidl_typesupport_cpp2/cmake/strip_verbatim.py) script is used after converting from .msg to .idl and before *rtiddsgen* is called to strip out instances of @verbatim annotations.
This script is only used for affected Connext versions.

RTI Connext issue: [CODEGENII-2200]
