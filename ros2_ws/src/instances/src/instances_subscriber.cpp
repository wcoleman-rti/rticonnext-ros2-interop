// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Copyright 2025 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdio>

#include <instances/msg/Instance.hpp>
#include <rclcpp/rclcpp.hpp>

namespace ros2_interop
{

class InstanceSubscriber : public rclcpp::Node
{
public:

  explicit InstanceSubscriber()
  : Node("instance_subscriber")
  {
    // Create a callback function for when messages are received.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    auto callback =
      [this](instances::msg::Instance::ConstSharedPtr msg) -> void
      {
        RCLCPP_INFO(this->get_logger(), "Received: [id: %ld] %s",msg->id, msg->msg.data());
        count_++;
      };

    sub_ = create_subscription<instances::msg::Instance>("instances",
        rclcpp::QoS(rclcpp::KeepLast(1)).reliable().transient_local(),
        callback);
  }

  void run()
  {
    RCLCPP_INFO(this->get_logger(), "Starting"
      " subscriber");

    while (rclcpp::ok()) {
      rclcpp::spin_some(shared_from_this());
    }
  }

  ~InstanceSubscriber() override
  {
    RCLCPP_INFO(this->get_logger(), "Finalized subscriber, received %d msgs", count_);
  }

private:
  rclcpp::Subscription<instances::msg::Instance>::SharedPtr sub_;
  uint32_t count_{0};
};

} // namespace ros2_interop



int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  std::make_shared<ros2_interop::InstanceSubscriber>()->run();
  rclcpp::shutdown();
  return 0;
}
