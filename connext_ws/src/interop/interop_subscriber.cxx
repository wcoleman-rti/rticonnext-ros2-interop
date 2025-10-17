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

#include <interop/msg/Interop.hpp>
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

namespace connext
{

class InteropSubscriber
{
public:

  explicit InteropSubscriber(unsigned int domain_id = 0)
  {
    // Create a callback function for when messages are received.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    auto callback =
      [this](const rti::sub::LoanedSample< ::interop::msg::Interop>& sample) -> void
      {
        if (sample.info().valid()
#ifdef USE_SHMEM_REF
              && sub_->is_data_consistent(sample)
#endif
            ) {
            const auto& msg = sample.data();
            fprintf(stdout, "Received: [id: %ld] %s\n", msg.id(), msg.msg().data());
            count_++;
        }
      };
    
    dds::domain::DomainParticipant participant(domain_id, dds::core::QosProvider::Default().participant_qos("QosLibrary::DefaultQos"));
    dds::topic::Topic< ::interop::msg::Interop> topic(participant, "rt/interop", "interop::msg::dds_::InteropMsg_");
    sub_ = dds::sub::DataReader< ::interop::msg::Interop>(
        topic,
        dds::core::QosProvider::Default().datareader_qos(
          "QosLibrary::DefaultQos"));
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

  ~InteropSubscriber()
  {
    fprintf(stdout, "Finalized subscriber, received %d msgs\n", count_);
  }

private:
  dds::sub::DataReader< ::interop::msg::Interop> sub_ = dds::core::null;
  rti::sub::SampleProcessor msg_processor_;
  uint32_t count_{0};
};

} // namespace connext



int main(int argc, char * argv[])
{
  setup_signal_handlers();
  (void)argc;
  (void)argv;
  std::make_shared<connext::InteropSubscriber>()->run();
  dds::domain::DomainParticipant::finalize_participant_factory();
  return 0;
}
