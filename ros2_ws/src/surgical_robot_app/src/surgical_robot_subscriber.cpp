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

#include <atomic>
#include <csignal>

#include <rclcpp/rclcpp.hpp>
#include <dds/dds.hpp>
#include <rti/rti.hpp>
#include <rti/sub/SampleProcessor.hpp>

#include <connext/surgical_robot_msgs/msg/SurgeryCommand.hpp>  // connextidl
#include <surgical_robot_msgs/msg/surgery_state.hpp>   // rosidl

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

class SurgicalSubscriber : public rclcpp::Node
{
public:

  explicit SurgicalSubscriber(unsigned int domain_id = 0)
  : Node("surgical_subscriber")
  {
    // Create a callback function for when messages are received.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    dds::domain::DomainParticipant participant(domain_id, dds::core::QosProvider::Default().participant_qos("QosLibrary::DefaultQos"));

    auto msg_callback =
      [this](const rti::sub::LoanedSample<surgical_robot_msgs::msg::SurgeryCommand>& sample) -> void
      {
        if (sample.info().valid()) {
          auto && msg = sample.data();
          RCLCPP_INFO(this->get_logger(), "Command Received:  [id: %s] %d", msg.header.frame_id.c_str(), msg.state_cmd);
          command_count_++;
        }
      };
    dds::topic::Topic<surgical_robot_msgs::msg::SurgeryCommand> command_topic(participant, "rt/SurgeryCommand", "surgical_robot_msgs::msg::dds_::surgery_command_");
    command_sub_ = dds::sub::DataReader<surgical_robot_msgs::msg::SurgeryCommand>(
      command_topic,
        dds::core::QosProvider::Default().datareader_qos(
          "QosLibrary::DefaultQos"));
    command_processor_.attach_reader(command_sub_, msg_callback);

    auto state_callback =
      [this](surgical_robot_msgs::msg::SurgeryState::ConstSharedPtr msg) -> void
      {
        RCLCPP_INFO(this->get_logger(), "State Received: [id: %s, state: %d, surgeon: %s]", msg->header.frame_id.c_str(), msg->state, msg->surgeon_name.c_str());
        state_count_++;
      };
    state_sub_ = create_subscription<surgical_robot_msgs::msg::SurgeryState>("SurgeryState",
        rclcpp::QoS(rclcpp::KeepLast(1)).best_effort(),
        state_callback);

    RCLCPP_INFO(this->get_logger(), "Starting subscriber");
    participant.enable();
  }

  ~SurgicalSubscriber() override
  {
    RCLCPP_INFO(this->get_logger(), "Finalized subscriber, received %d msgs", command_count_);
  }

private:
  dds::sub::DataReader<surgical_robot_msgs::msg::SurgeryCommand> command_sub_ = nullptr;
  rclcpp::Subscription<surgical_robot_msgs::msg::SurgeryState>::SharedPtr state_sub_ = nullptr;
  rti::sub::SampleProcessor command_processor_;
  uint32_t command_count_ = 0;
  uint32_t state_count_ = 0;
};

} // namespace mixed



int main(int argc, char * argv[])
{
  setup_signal_handlers();
  rclcpp::init(argc, argv);
  auto app = std::make_shared<mixed::SurgicalSubscriber>();
  while (!shutdown_requested.load() && rclcpp::ok()) {
    rclcpp::spin_some(app);
  }
  rclcpp::shutdown();
  printf("Shutdown complete.\n");
  return 0;
}
