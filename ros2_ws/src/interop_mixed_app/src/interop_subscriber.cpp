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

#include <connext/interop_interface/msg/Interop.hpp>  // connextidl
#include <interop_interface/msg/status.hpp>   // rosidl

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

class InteropSubscriber : public rclcpp::Node
{
public:

  explicit InteropSubscriber(unsigned int domain_id = 0)
  : Node("interop_subscriber")
  {
    // Create a callback function for when messages are received.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    dds::domain::DomainParticipant participant(domain_id, dds::core::QosProvider::Default().participant_qos("QosLibrary::DefaultQos"));

    auto msg_callback =
      [this](const rti::sub::LoanedSample<interop_interface::msg::Interop>& sample) -> void
      {
        if (sample.info().valid()) {
          auto && msg = sample.data();
          RCLCPP_INFO(this->get_logger(), "Msg Received: [id: %ld] %s", msg.id(), msg.msg().data());
          count_++;
        }
      };
    dds::topic::Topic<interop_interface::msg::Interop> msg_topic(participant, "rt/interop", "interop_interface::msg::dds_::InteropMsg_");
    msg_sub_ = dds::sub::DataReader<interop_interface::msg::Interop>(
      msg_topic,
        dds::core::QosProvider::Default().datareader_qos(
          "QosLibrary::DefaultQos"));
    msg_processor_.attach_reader(msg_sub_, msg_callback);

    auto status_callback =
      [this](interop_interface::msg::Status::ConstSharedPtr msg) -> void
      {
        RCLCPP_INFO(this->get_logger(), "Status Received: [id: %ld, count: %ld]", msg->id, msg->msg_count);
        count_++;
      };
    status_sub_ = create_subscription<interop_interface::msg::Status>("interop_status",
        rclcpp::QoS(rclcpp::KeepLast(1)).best_effort(),
        status_callback);

    RCLCPP_INFO(this->get_logger(), "Starting subscriber");
    participant.enable();
  }

  ~InteropSubscriber() override
  {
    RCLCPP_INFO(this->get_logger(), "Finalized subscriber, received %d msgs", count_);
  }

private:
  dds::sub::DataReader<interop_interface::msg::Interop> msg_sub_ = nullptr;
  rclcpp::Subscription<interop_interface::msg::Status>::SharedPtr status_sub_ = nullptr;
  rti::sub::SampleProcessor msg_processor_;
  uint32_t count_ = 0;
};

} // namespace mixed



int main(int argc, char * argv[])
{
  setup_signal_handlers();
  rclcpp::init(argc, argv);
  auto app = std::make_shared<mixed::InteropSubscriber>();
  while (!shutdown_requested.load() && rclcpp::ok()) {
    rclcpp::spin_some(app);
  }
  rclcpp::shutdown();
  printf("Shutdown complete.\n");
  return 0;
}
