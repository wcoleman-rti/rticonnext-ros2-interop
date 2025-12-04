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
#include <csignal>
#include <atomic>

#include <interop_interface/msg/Interop.hpp>
#include <dds/dds.hpp>
#include <rti/rti.hpp>

std::atomic<bool> shutdown_requested{false};

inline void stop_handler(int)
{
    shutdown_requested.store(true);
    fprintf(stdout, "preparing to shut down...\n");
}

inline void setup_signal_handlers()
{
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
    dds::topic::Topic<interop_interface::msg::Interop> topic(participant, "rt/interop", "interop_interface::msg::dds_::InteropMsg_");
    pub_ = dds::pub::DataWriter<interop_interface::msg::Interop>(
        topic, 
        dds::core::QosProvider::Default().datawriter_qos(
          "QosLibrary::DefaultQos"));
  }

  void run()
  {
    fprintf(stdout, "Starting" 
        " (SHMEM_REF)"
        " publisher with id %d\n", id_);

    std::string user_input;
    while (!shutdown_requested.load()) {
      std::cout << "> ";
      // Use std::getline to read the entire line, including spaces, until Enter is pressed
      if (std::getline(std::cin, user_input))
      {
          // Only publish if the input is not empty
          if (!user_input.empty())
          {
            auto interop_msg = pub_->get_loan();
            interop_msg->id(id_);
            strncpy(reinterpret_cast<char*>(interop_msg->msg().data()), user_input.c_str(), sizeof(interop_msg->msg()) - 1);
            interop_msg->msg().data()[sizeof(interop_msg->msg()) - 1] = '\0'; // Ensure null termination
            fprintf(stdout, "Publishing: [id: %ld] %s\n", interop_msg->id(), interop_msg->msg().data());
            pub_->write(*interop_msg);
            count_++;
          }
      }
      else
      {
          // Handle EOF (e.g., Ctrl+D or end of pipe)
          fprintf(stdout, "Input stream closed. Exiting...\n");
          stop_handler(SIGTERM);
          break;
      }
    }
  }

  ~InteropPublisher()
  {
    fprintf(stdout, "Finalized publisher with id %d, published %d msgs\n", id_, count_);
  }

private:
  uint16_t id_{0};
  dds::pub::DataWriter<interop_interface::msg::Interop> pub_ = dds::core::null;
  uint32_t count_{0};
};

} // namespace connext

int main(int argc, char * argv[])
{
  setup_signal_handlers();
  int id = 0;
  if (argc > 1) {
    id = std::atoi(argv[1]);
  }
  rti::util::network_capture::enable();
  rti::util::network_capture::start("capture");
  std::make_shared<connext::InteropPublisher>(id)->run();
  rti::util::network_capture::stop();
  rti::util::network_capture::disable();
  dds::domain::DomainParticipant::finalize_participant_factory();
  printf("Shutdown complete.\n");
  return 0;
}
