/*
* (c) Copyright, Real-Time Innovations, 2020.  All rights reserved.
* RTI grants Licensee a license to use, modify, compile, and create derivative
* works of the software solely for use with RTI Connext DDS. Licensee may
* redistribute copies of the software provided that all such copies are subject
* to this license. The software is provided "as is", with no warranty of any
* type, including any warranty for fitness for any purpose. RTI is under no
* obligation to maintain or support the software. RTI shall not be liable for
* any incidental or consequential damages arising out of the use or inability
* to use the software.
*/

#include <cstdint>
#include <cstdio>
#include <memory>
#include <utility>
#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>

#include <dds/dds.hpp>
#include <rti/rti.hpp>
#include <rti/sub/SampleProcessor.hpp>

#include <connext/interop_interface/msg/Interop.hpp>  // connextidl
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

namespace connext
{

class InteropSubscriber
{
public:

  explicit InteropSubscriber(unsigned int domain_id = 0)
  {
    // Create a callback function for when messages are received.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    dds::domain::DomainParticipant participant(domain_id, dds::core::QosProvider::Default().participant_qos("QosLibrary::DefaultQos"));

    auto msg_callback =
      [this](const rti::sub::LoanedSample<interop_interface::msg::Interop>& sample) -> void
      {
        if (sample.info().valid()) {
          auto && msg = sample.data();
          fprintf(stdout, "Msg Received: [id: %ld] %s\n", msg.id(), msg.msg().data());
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
      [this](const rti::sub::LoanedSample<interop_interface::msg::Status>& sample) -> void
      {
        if (sample.info().valid()) {
          auto && msg = sample.data();
          fprintf(stdout, "Status Received: [id: %ld, count: %ld]\n", msg.id(), msg.msg_count());
          count_++;
        }
      };
    dds::topic::Topic<interop_interface::msg::Status> status_topic(participant, "rt/interop_status", "interop_interface::msg::dds_::StatusMsg_");
    status_sub_ = dds::sub::DataReader<interop_interface::msg::Status>(
      status_topic,
        dds::core::QosProvider::Default().datareader_qos(
          "QosLibrary::DefaultQos"));
    msg_processor_.attach_reader(status_sub_, status_callback);

    fprintf(stdout, "Starting subscriber\n");
    participant.enable();
  }

  ~InteropSubscriber()
  {
    fprintf(stdout, "Finalized subscriber, received %d msgs\n", count_);
  }

private:
  dds::sub::DataReader<interop_interface::msg::Interop> msg_sub_ = nullptr;
  dds::sub::DataReader<interop_interface::msg::Status> status_sub_ = nullptr;
  rti::sub::SampleProcessor msg_processor_;
  uint32_t count_ = 0;
};

} // namespace connext



int main(int argc, char * argv[])
{
  setup_signal_handlers();
  (void)argc;
  (void)argv;
  auto app = std::make_shared<connext::InteropSubscriber>();
  while (!shutdown_requested.load()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  app.reset();
  printf("Shutdown complete.\n");
  return 0;
}
