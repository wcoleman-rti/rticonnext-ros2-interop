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

#include <interop_interface/msg/Interop.hpp>
#include <rclcpp/rclcpp.hpp>

namespace ros2
{

class InteropSubscriber : public rclcpp::Node
{
public:

  explicit InteropSubscriber()
  : Node("interop_subscriber")
  {
    // Create a callback function for when messages are received.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    auto callback =
      [this](interop_interface::msg::Interop::ConstSharedPtr msg) -> void
      {
        RCLCPP_INFO(this->get_logger(), "Received: [id: %ld] %s",msg->id, msg->msg.data());
        count_++;
      };

    sub_ = create_subscription<interop_interface::msg::Interop>("interop",
        rclcpp::QoS(rclcpp::KeepLast(100)).best_effort(),
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

  ~InteropSubscriber() override
  {
    RCLCPP_INFO(this->get_logger(), "Finalized subscriber, received %d msgs", count_);
  }

private:
  rclcpp::Subscription<interop_interface::msg::Interop>::SharedPtr sub_;
  uint32_t count_{0};
};

} // namespace ros2



int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  std::make_shared<ros2::InteropSubscriber>()->run();
  rclcpp::shutdown();
  return 0;
}
