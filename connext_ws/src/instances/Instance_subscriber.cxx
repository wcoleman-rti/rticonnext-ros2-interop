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

#include <cstdio>
#include <iostream>
#include <csignal>
#include <atomic>

#include <instances/msg/Instance.hpp>
#include <dds/dds.hpp>
#include <rti/rti.hpp>
#include <rti/sub/SampleProcessor.hpp>

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

namespace ros2_interop
{

class InstanceSubscriber
{
public:

  explicit InstanceSubscriber(unsigned int domain_id = 0)
  {
    // Create a callback function for when messages are received.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    auto callback =
      [this](const rti::sub::LoanedSample< ::instances::msg::Instance>& sample) -> void
      {
        if (sample.info().valid()) {
            const auto& msg = sample.data();
            fprintf(stdout, "Received: [id: %ld] %s\n", msg.id(), msg.msg().data());
            count_++;
        }
      };
    
    dds::domain::DomainParticipant participant(domain_id);
    dds::topic::Topic< ::instances::msg::Instance> topic(participant, "rt/instances", "instances::msg::dds_::Instance_");
    sub_ = dds::sub::DataReader< ::instances::msg::Instance>(
        topic, 
        dds::core::QosProvider::Default().datareader_qos(
            rti::core::builtin_profiles::qos_lib::generic_keep_last_reliable_transient_local()));
    msg_processor_.attach_reader(sub_, callback);
  }

  void run()
  {
    fprintf(stdout, "Starting" 
        #ifdef USE_SHMEM_REF
        " (SHMEM_REF)"
        #endif
        " subscriber\n");

    while (!shutdown_requested.load()) {
        rti::util::sleep(dds::core::Duration(1));
    }
  }

  ~InstanceSubscriber()
  {
    fprintf(stdout, "Finalized subscriber, received %d msgs\n", count_);
  }

private:
  dds::sub::DataReader< ::instances::msg::Instance> sub_ = dds::core::null;
  rti::sub::SampleProcessor msg_processor_;
  uint32_t count_{0};
};

} // namespace ros2_interop



int main(int argc, char * argv[])
{
  setup_signal_handlers();
  (void)argc;
  (void)argv;
  std::make_shared<ros2_interop::InstanceSubscriber>()->run();
  dds::domain::DomainParticipant::finalize_participant_factory();
  return 0;
}
