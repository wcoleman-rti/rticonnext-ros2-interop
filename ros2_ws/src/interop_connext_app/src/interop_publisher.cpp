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

class InteropPublisher
{
public:

  explicit InteropPublisher(int id = 0, unsigned int domain_id = 0) 
  : id_(id)
  {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    dds::domain::DomainParticipant participant(domain_id, dds::core::QosProvider::Default().participant_qos("QosLibrary::DefaultQos"));

    dds::topic::Topic<interop_interface::msg::Interop> msg_topic(participant, "rt/interop", "interop_interface::msg::dds_::InteropMsg_");
    msg_pub_ = dds::pub::DataWriter<interop_interface::msg::Interop>(
      msg_topic, 
        dds::core::QosProvider::Default().datawriter_qos(
          "QosLibrary::DefaultQos"));
    
          dds::topic::Topic<interop_interface::msg::Status> status_topic(participant, "rt/interop_status", "interop_interface::msg::dds_::StatusMsg_");
    status_pub_ = dds::pub::DataWriter<interop_interface::msg::Status>(
        status_topic, 
        dds::core::QosProvider::Default().datawriter_qos(
          "QosLibrary::DefaultQos"));
    
    participant.enable();

    input_thread_ = std::thread(&InteropPublisher::run, this);

    while (!shutdown_requested.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  void run()
  {
    fprintf(stdout, "Starting" 
        " publisher with id %d\n", id_);

    std::string user_input;
    while (!stop_thread_.load()) {
      std::cout << "> ";
      // Use std::getline to read the entire line, including spaces, until Enter is pressed
      if (std::getline(std::cin, user_input))
      {
          // Only publish if the input is not empty
          if (!user_input.empty())
          {
            auto interop_msg = std::make_unique<interop_interface::msg::Interop>();
            interop_msg->id(id_);
            strncpy(reinterpret_cast<char*>(interop_msg->msg().data()), user_input.c_str(), sizeof(interop_msg->msg()) - 1);
            interop_msg->msg()[sizeof(interop_msg->msg()) - 1] = '\0'; // Ensure null termination
            fprintf(stdout, "Publishing Msg: [id: %ld] %s\n", interop_msg->id(), interop_msg->msg().data());
            msg_pub_.write(*interop_msg);
            count_++;

            if (count_ % 4 == 0) {
              auto status_msg = std::make_unique<interop_interface::msg::Status>();
              status_msg->id(id_);
              status_msg->msg_count(count_);
              fprintf(stdout, "Publishing Status: [id: %ld, count: %ld]\n", status_msg->id(), status_msg->msg_count());
              status_pub_.write(*status_msg);
            }
          }
      }
      else
      {
          // Handle EOF (e.g., Ctrl+D or end of pipe)
          fprintf(stdout, "Input stream closed. Exiting...\n");
          shutdown_requested.store(true);
          break;
      }
    }
  }

  ~InteropPublisher()
  {
    fprintf(stdout, "Shutting down publisher with id %d\n", id_);
    if (input_thread_.joinable()) {
      stop_thread_.store(true);
      input_thread_.join();
    }
    fprintf(stdout, "Finalized publisher with id %d, published %d msgs\n", id_, count_);
  }

private:
  uint16_t id_;
  dds::pub::DataWriter<interop_interface::msg::Interop> msg_pub_ = nullptr;
  dds::pub::DataWriter<interop_interface::msg::Status> status_pub_ = nullptr;
  uint32_t count_ = 0;
  std::atomic<bool> stop_thread_ = false;
  std::thread input_thread_;
};

} // namespace connext

int main(int argc, char * argv[])
{
  setup_signal_handlers();
  int id = 0;
  if (argc > 1) {
    id = std::atoi(argv[1]);
  }
  std::make_shared<connext::InteropPublisher>(id);
  printf("Shutdown complete.\n");
  return 0;
}
