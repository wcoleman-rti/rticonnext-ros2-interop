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

#include <interop_interface/msg/interop.hpp>          // rosidl
#include <connext/interop_interface/msg/Status.hpp>   // connextidl

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

class InteropPublisher : public rclcpp::Node
{
public:

  explicit InteropPublisher(int id = 0, unsigned int domain_id = 0)
  : Node("interop_publisher"), id_(id)
  {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    dds::domain::DomainParticipant participant(domain_id, dds::core::QosProvider::Default().participant_qos("QosLibrary::DefaultQos"));

    msg_pub_ = this->create_publisher<interop_interface::msg::Interop>(
        "interop",
        rclcpp::QoS(rclcpp::KeepLast(1)).best_effort());
    
    dds::topic::Topic<interop_interface::msg::Status> status_topic(participant, "rt/interop_status", "interop_interface::msg::dds_::StatusMsg_");
    status_pub_ = dds::pub::DataWriter<interop_interface::msg::Status>(
        status_topic, 
        dds::core::QosProvider::Default().datawriter_qos(
          "QosLibrary::DefaultQos"));
    
    participant.enable();

    input_thread_ = std::thread(&InteropPublisher::run, this);
  }

  void run()
  {
    RCLCPP_INFO(this->get_logger(), "Starting"
      " publisher with id %d", id_);

    std::string user_input;
    while (!stop_thread_.load() && rclcpp::ok()) {
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
            RCLCPP_INFO(this->get_logger(), "Publishing Msg: [id: %ld] %s", interop_msg->id, interop_msg->msg.data());
            msg_pub_->publish(std::move(interop_msg));
            count_++;

            if (count_ % 4 == 0) {
              auto status_msg = std::make_unique<interop_interface::msg::Status>();
              status_msg->id(id_);
              status_msg->msg_count(count_);
              RCLCPP_INFO(this->get_logger(), "Publishing Status: [id: %ld, count: %ld]", status_msg->id(), status_msg->msg_count());
              status_pub_.write(*status_msg);
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

  ~InteropPublisher() override
  {
    RCLCPP_INFO(this->get_logger(), "Shutting down publisher with id %d", id_);
    if (input_thread_.joinable()) {
      stop_thread_.store(true);
      input_thread_.join();
    }
    RCLCPP_INFO(this->get_logger(), "Finalized publisher with id %d, published %d msgs", id_, count_);
  }

private:
  uint16_t id_;
  rclcpp::Publisher<interop_interface::msg::Interop>::SharedPtr msg_pub_ = nullptr;
  dds::pub::DataWriter<interop_interface::msg::Status> status_pub_ = nullptr;
  uint32_t count_ = 0;
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
  auto app = std::make_shared<mixed::InteropPublisher>(id);
  while (!shutdown_requested.load() && rclcpp::ok()) {
    rclcpp::spin_some(app);
  }
  rclcpp::shutdown();
  printf("Shutdown complete.\n");
  return 0;
}
