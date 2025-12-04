# Native Connext - ROS2 rmw_connextdds Interop

## Overview

This set of examples demonstrates interoperability between:

1. ROS Humble pub/sub apps using rmw_connextdds
2. RTI Connext 7.3.0+ pub/sub apps

It also demonstrates features that have interoperability considerations such as:

1. [Zero Copy over SHMEM](#zero-copy)

## Requirements

1. [ROS2 Humble](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debs.html#install-ros-2)
    * [RTI Connext 6.0.1](https://docs.ros.org/en/humble/Installation/RMW-Implementations/DDS-Implementations/Working-with-RTI-Connext-DDS.html#rti-connext-dds) is the default RMW RTI provides for this ROS2 release.
2. [RTI Connext 7.3.0+](https://community.rti.com/static/documentation/connext-dds/current/doc/manuals/debian_packages/install.html)

## Build

*Note: you should setup your RTI Connext build environment to be against the version the native connext app will link against, regardless of the version the ROS2 RMW app will use.*

```sh
source /opt/ros/humble/setup.sh
source /opt/rti.com/rti_connext_dds-7.3.0/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.bash
cd ros2_ws
colcon build --symlink-install
```

## Run

### ROS2 Run Apps

Common environment:

```sh
# Setup the ROS2 environment
cd ros2_ws
source install/setup.bash

# Use rmw_connextdds and setup shared Connext libs
source /opt/rti.com/rti_connext_dds-6.0.1/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.bash
export RMW_IMPLEMENTATION=rmw_connextdds
export NDDS_QOS_PROFILES=install/interop_interface/share/interop_interface/config/USER_QOS_PROFILES.xml
```

Publisher:

```sh
ros2 run interop_ros2_app interop_publisher 3  # <3> is used as the key 'id' field value of data published
```

Subscriber:

```sh
ros2 run interop_ros2_app interop_subscriber
```

### Connext Run Apps

Common environment:

```sh
# Setup the ROS2 environment
cd ros2_ws
source install/setup.bash

# Setup shared Connext libs
source /opt/rti.com/rti_connext_dds-7.3.0/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.bash
export NDDS_QOS_PROFILES=install/interop_interface/share/interop_interface/config/USER_QOS_PROFILES.xml
```

Publisher:

```sh
ros2 run interop_connext_app interop_publisher 4  # <4> is used as the key 'id' field value of data published
```

Subscriber:

```sh
ros2 run interop_connext_app interop_subscriber
```

## Features

### Zero Copy

ROS2 cannot leverage RTI Connext Zero Copy with the rmw_connextdds.
However it can interoperate - it just means that a ZC Connext writer will write plain data to a ROS2 reader. No other api changes or configuration should be needed.

See:

* [Connext: Zero Copy over SHMEM](https://community.rti.com/static/documentation/connext-dds/7.3.0/doc/manuals/connext_dds/html_files/RTI_ConnextDDS_CoreLibraries_UsersManual/index.htm#UsersManual/SendingLDZeroCopyUsing.htm)

#### Expected interoperability results

|                         | Connext Pub (Zero Copy) | ROS2 Pub |
|-------------------------|-------------------------|----------|
| Connext Sub (Zero Copy) | `SHMEMREF`              | `PLAIN`  |
| ROS2 Sub                | `PLAIN`                 | `PLAIN`  |

## Issues

1. Ament build warning for `interop_interface` package.

    ```none
    CMake Warning at <...>/ros2_ws/install/interop_interface/share/interop_interface/cmake/rosidl_cmake_export_typesupport_targets-extras.cmake:18 (message):
        Package 'interop_interface' exports the typesupport target
        'interop_interface::interop_interface__rosidl_typesupport_cpp' which
        doesn't exist
    Call Stack (most recent call first):
        <...>/ros2_ws/install/interop_interface/share/interop_interface/cmake/interop_interfaceConfig.cmake:41 (include)
        CMakeLists.txt:11 (find_package)
    ```

    The build succeeds and the `interop_ros2_app` package builds successfully, linking against the `interop_interface::rosidl_typesupport_cpp` exported target from this package.

    This should be reviewed further.

2. Double free for `interop_connext_app` applications on shutdown.

    ```none
    Shutdown complete.
    double free or corruption (fasttop)
    [ros2run]: Aborted
    ```

    This may have something to do with a dynamic linking issue if both 7.3.0 (from native connext build of `interop_interface`) and 6.0.1 (from `rmw_connextdds`) are both being linked.

    This should be reviewed further.
