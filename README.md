# Native Connext - ROS2 rmw_connextdds Interop

## Overview

This set of examples demonstrates interoperability between:

1. ROS Kilted pub/sub apps using rmw_connextdds
2. RTI Connext 7.3.0 pub/sub apps

It also demonstrates features that have interoperability considerations such as:

1. [Keys & Instances](#keys--instances)
2. [Zero Copy over SHMEM](#zero-copy)

## Requirements

1. [ROS2 Kilted](https://docs.ros.org/en/kilted/Installation/Ubuntu-Install-Debs.html#install-ros-2)
2. [RTI Connext 7.3.0](https://docs.ros.org/en/kilted/Installation/RMW-Implementations/DDS-Implementations/Working-with-RTI-Connext-DDS.html#rti-connext-dds)

## Build Apps

### ROS2 Build Apps

```sh
source /opt/ros/kilted/setup.sh
cd ros2_ws
colcon build --symlink-install
```

### Connext Build Apps

```sh
source /opt/rti.com/rti_connext_dds-7.3.0/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.bash
cd connext_ws
cmake -B build
cmake --build build
cmake --install build
```

Optionally, to build the Connext apps to use Zero Copy (SHMEMREF):

```sh
source /opt/rti.com/rti_connext_dds-7.3.0/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.bash
cd connext_ws
cmake -B build -DUSE_SHMEM_REF=1
cmake --build build
cmake --install build
```

## Run Apps

### ROS2 Run Apps

Common environment:

```sh
# Setup the ROS2 environment
cd ros2_ws
source install/setup.bash

# Use rmw_connextdds and setup shared Connext libs
source /opt/rti.com/rti_connext_dds-7.3.0/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.bash
export RMW_IMPLEMENTATION=rmw_connextdds
```

Publisher:

```sh
ros2 run instances instances_subscriber 5  # <5> is used as the key 'id' field value of data published
```

Subscriber:

```sh
ros2 run instances instances_subscriber
```

### Connext Run Apps

Common environment:

```sh
# Setup shared Connext libs
cd connext_ws/bin
source /opt/rti.com/rti_connext_dds-7.3.0/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.bash
```

Publisher:

```sh
./Instances_publisher 5  # <5> is used as the key 'id' field value of data published
```

Subscriber:

```sh
./Instances_subscriber
```

## Features

### Keys & Instances

ROS2 supports annotating datatype files as key fields starting in Kilted.

See:

* [ROS2: Topic Keys Tutorial](https://docs.ros.org/en/kilted/Tutorials/Advanced/Topic-Keys/Topic-Keys-Tutorial.html#topic-keys-tutorial)

### Zero Copy

ROS2 cannot leverage RTI Connext Zero Copy with the rmw_connextdds.
However it can interoperate - it just means that a ZC Connext writer will write plain data to a ROS2 reader. No other api changes or configuration should be needed.

See:

* [Connext: Zero Copy over SHMEM](https://community.rti.com/static/documentation/connext-dds/7.3.0/doc/manuals/connext_dds_professional/users_manual/users_manual/SendingLDZeroCopy.htm)

#### Expected interoperability results

|                         | Connext Pub (Zero Copy) | Connext Pub (Plain) | ROS2 Pub |
|-------------------------|-------------------------|---------------------|----------|
| Connext Sub (Zero Copy) | `SHMEMREF`              | `PLAIN`             | `PLAIN`  |
| Connext Sub (Plain)     | `PLAIN`                 | `PLAIN`             | `PLAIN`  |
| ROS2 Sub                | `PLAIN`                 | `PLAIN`             | `PLAIN`  |
