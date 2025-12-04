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

#include <cstdint>
#include <cstdio>
#include <memory>
#include <utility>
#include <iostream>

#include <interop_interface/msg/Interop.hpp>
#include <rclcpp/rclcpp.hpp>

namespace ros2
{

class InteropPublisher : public rclcpp::Node
{
public:

  explicit InteropPublisher(int id = 0)
  : Node("interop_publisher"), id_(id)
  {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    pub_ = this->create_publisher<interop_interface::msg::Interop>(
        "interop",
        rclcpp::QoS(rclcpp::KeepLast(1)).best_effort());
  }

  void run()
  {
    RCLCPP_INFO(this->get_logger(), "Starting"
      " publisher with id %d", id_);

    std::string user_input;
    while (rclcpp::ok()) {
      std::cout << "> ";
      // Use std::getline to read the entire line, including spaces, until Enter is pressed
      if (std::getline(std::cin, user_input))
      {
          // Only publish if the input is not empty
          if (!user_input.empty())
          {
            auto interop_msg = std::make_unique<interop_interface::msg::Interop>();
            interop_msg->id = id_;
            strncpy(reinterpret_cast<char*>(interop_msg->msg.data()), user_input.c_str(), sizeof(interop_msg->msg) - 1);
            interop_msg->msg[sizeof(interop_msg->msg) - 1] = '\0'; // Ensure null termination
            RCLCPP_INFO(this->get_logger(), "Publishing: [id: %ld] %s", interop_msg->id, interop_msg->msg.data());
            pub_->publish(std::move(interop_msg));
            count_++;
          }
      }
      else
      {
          // Handle EOF (e.g., Ctrl+D or end of pipe)
          RCLCPP_INFO(this->get_logger(), "Input stream closed. Exiting...");
          break;
      }

      // Allow the ROS 2 executor to process the publication
      rclcpp::spin_some(shared_from_this());
    }
  }

  ~InteropPublisher() override
  {
    RCLCPP_INFO(this->get_logger(), "Finalized publisher with id %d, published %d msgs", id_, count_);
  }

private:
  uint16_t id_;
  rclcpp::Publisher<interop_interface::msg::Interop>::SharedPtr pub_;
  uint32_t count_{0};
};

} // namespace ros2

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  int id = 0;
  if (argc > 1) {
    id = std::atoi(argv[1]);
  }
  std::make_shared<ros2::InteropPublisher>(id)->run();
  rclcpp::shutdown();
  return 0;
}
