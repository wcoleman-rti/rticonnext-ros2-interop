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
#include <thread>
#include <atomic>
#include <csignal>

#include <rclcpp/rclcpp.hpp>
#include <dds/dds.hpp>
#include <rti/rti.hpp>

#include <surgical_robot_msgs/msg/surgery_command.hpp>          // rosidl
#include <connext/surgical_robot_msgs/msg/SurgeryState.hpp>   // connextidl

std::atomic<bool> shutdown_requested{false};

inline void setup_signal_handlers()
{
  auto stop_handler = [](int) {
      shutdown_requested.store(true);
      fprintf(stdout, "preparing to shut down...\n");
  };
  signal(SIGINT, stop_handler);
  signal(SIGTERM, stop_handler);
}

namespace mixed
{

class SurgicalPublisher : public rclcpp::Node
{
public:

  explicit SurgicalPublisher(int id = 0, unsigned int domain_id = 0)
  : Node("surgical_publisher"), id_(id)
  {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    dds::domain::DomainParticipant participant(domain_id, dds::core::QosProvider::Default().participant_qos("QosLibrary::DefaultQos"));

    command_pub_ = this->create_publisher<surgical_robot_msgs::msg::SurgeryCommand>(
        "SurgeryCommand",
        rclcpp::QoS(rclcpp::KeepLast(1)).best_effort());
    
    dds::topic::Topic<surgical_robot_msgs::msg::SurgeryState> state_topic(participant, "rt/SurgeryState", "surgical_robot_msgs::msg::dds_::surgery_state_");
    state_pub_ = dds::pub::DataWriter<surgical_robot_msgs::msg::SurgeryState>(
        state_topic, 
        dds::core::QosProvider::Default().datawriter_qos(
          "QosLibrary::DefaultQos"));
    
    participant.enable();

    input_thread_ = std::thread(&SurgicalPublisher::run, this);
  }

  void run()
  {
    RCLCPP_INFO(this->get_logger(), "Starting"
      " publisher with id %d", id_);

    current_state_ = surgical_robot_msgs::msg::SurgeryCommand::NOT_DOING_SURGERY;

    std::string user_input;
    while (!stop_thread_.load() && rclcpp::ok()) {
      std::cout << "> ";
      // Use std::getline to read the entire line, including spaces, until Enter is pressed
      if (std::getline(std::cin, user_input))
      {
          // Only publish if the input is not empty
          if (!user_input.empty())
          {
            if (user_input == "stop") {
              current_state_ = surgical_robot_msgs::msg::SurgeryCommand::NOT_DOING_SURGERY;
            } else if (user_input == "start") {
              current_state_ = surgical_robot_msgs::msg::SurgeryCommand::DOING_SURGERY;
            }

            auto command_msg = std::make_unique<surgical_robot_msgs::msg::SurgeryCommand>();
            command_msg->header.frame_id = std::to_string(command_count_);
            // strncpy(command_msg->header.frame_id, std::to_string(command_count_).c_str(), sizeof(command_msg->header.frame_id) - 1);
            // command_msg->header.frame_id[sizeof(command_msg->header.frame_id) - 1] = '\0'; // Ensure null termination
            command_msg->state_cmd = current_state_;
            
            RCLCPP_INFO(this->get_logger(), "Publishing Command: [id: %s] %d", command_msg->header.frame_id.c_str(), command_msg->state_cmd);
            command_pub_->publish(std::move(command_msg));
            command_count_++;

            if (command_count_ % 4 == 0) {
              auto status_msg = std::make_unique<surgical_robot_msgs::msg::SurgeryState>();
              status_msg->header.frame_id = std::to_string(state_count_);
              status_msg->state = surgical_robot_msgs::msg::SurgeryCommand::DOING_SURGERY;
              status_msg->surgeon_name = "Dr. ROS2 #" + std::to_string(id_);

              RCLCPP_INFO(this->get_logger(), "Publishing State: [id: %s, state: %d, surgeon: %s]", status_msg->header.frame_id.c_str(), status_msg->state, status_msg->surgeon_name.c_str());
              state_pub_.write(*status_msg);
              state_count_++;
            }
          }
      }
      else
      {
          // Handle EOF (e.g., Ctrl+D or end of pipe)
          RCLCPP_INFO(this->get_logger(), "Input stream closed. Exiting...");
          shutdown_requested.store(true);
          break;
      }
    }
  }

  ~SurgicalPublisher() override
  {
    RCLCPP_INFO(this->get_logger(), "Shutting down publisher with id %d", id_);

    // Publish a final status message before shutting down
    current_state_ = surgical_robot_msgs::msg::SurgeryCommand::NOT_DOING_SURGERY;
    auto status_msg = std::make_unique<surgical_robot_msgs::msg::SurgeryState>();
    status_msg->header.frame_id = std::to_string(state_count_);
    status_msg->state = current_state_;
    status_msg->surgeon_name = "Dr. ROS2 #" + std::to_string(id_);

    RCLCPP_INFO(this->get_logger(), "Publishing State: [id: %s, state: %d, surgeon: %s]", status_msg->header.frame_id.c_str(), status_msg->state, status_msg->surgeon_name.c_str());
    state_pub_.write(*status_msg);
    state_count_++;

    if (input_thread_.joinable()) {
      stop_thread_.store(true);
      input_thread_.join();
    }
    RCLCPP_INFO(this->get_logger(), "Finalized publisher with id %d, published %d msgs", id_, command_count_);
  }

private:
  uint16_t id_;
  rclcpp::Publisher<surgical_robot_msgs::msg::SurgeryCommand>::SharedPtr command_pub_ = nullptr;
  dds::pub::DataWriter<surgical_robot_msgs::msg::SurgeryState> state_pub_ = nullptr;
  uint32_t command_count_ = 0;
  uint32_t state_count_ = 0;
  uint8_t current_state_ = surgical_robot_msgs::msg::SurgeryCommand::NOT_DOING_SURGERY;
  std::atomic<bool> stop_thread_ = false;
  std::thread input_thread_;
};

} // namespace mixed

int main(int argc, char * argv[])
{
  setup_signal_handlers();
  rclcpp::init(argc, argv);
  int id = 0;
  if (argc > 1) {
    id = std::atoi(argv[1]);
  }
  auto app = std::make_shared<mixed::SurgicalPublisher>(id);
  while (!shutdown_requested.load() && rclcpp::ok()) {
    rclcpp::spin_some(app);
  }
  rclcpp::shutdown();
  printf("Shutdown complete.\n");
  return 0;
}
