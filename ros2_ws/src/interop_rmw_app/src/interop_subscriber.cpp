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
#include <rclcpp/rclcpp.hpp>

#include <interop_interface/msg/interop.hpp>  // rosidl
#include <interop_interface/msg/status.hpp>   // rosidl

namespace rmw
{

class InteropSubscriber : public rclcpp::Node
{
public:

  explicit InteropSubscriber()
  : Node("interop_subscriber")
  {
    // Create a callback function for when messages are received.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    auto msg_callback =
      [this](interop_interface::msg::Interop::ConstSharedPtr msg) -> void
      {
        RCLCPP_INFO(this->get_logger(), "Msg Received: [id: %ld] %s",msg->id, msg->msg.data());
        count_++;
      };
    msg_sub_ = create_subscription<interop_interface::msg::Interop>("interop",
        rclcpp::QoS(rclcpp::KeepLast(100)).best_effort(),
        msg_callback);
    
    auto status_callback =
      [this](interop_interface::msg::Status::ConstSharedPtr msg) -> void
      {
        RCLCPP_INFO(this->get_logger(), "Status Received: [id: %ld, count: %ld]",msg->id, msg->msg_count);
        count_++;
      };
    status_sub_ = create_subscription<interop_interface::msg::Status>("interop_status",
        rclcpp::QoS(rclcpp::KeepLast(1)).best_effort(),
        status_callback);
    
    RCLCPP_INFO(this->get_logger(), "Starting subscriber");
  }

  ~InteropSubscriber() override
  {
    RCLCPP_INFO(this->get_logger(), "Finalized subscriber, received %d msgs", count_);
  }

private:
  rclcpp::Subscription<interop_interface::msg::Interop>::SharedPtr msg_sub_ = nullptr;
  rclcpp::Subscription<interop_interface::msg::Status>::SharedPtr status_sub_ = nullptr;
  uint32_t count_ = 0;
};

} // namespace rmw



int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<rmw::InteropSubscriber>());
  rclcpp::shutdown();
  printf("Shutdown complete.\n");
  return 0;
}
