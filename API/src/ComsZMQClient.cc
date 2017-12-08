
#include "../include/dogbot/ComsZMQClient.hh"
#include "dogbot/ZMQContext.hh"

namespace DogBotN
{

  ComsZMQClientC::ComsZMQClientC(const std::string &portAddr)
  {
    Open(portAddr);
  }

  //! default
  ComsZMQClientC::ComsZMQClientC()
  {}

  // Disconnects and closes file descriptors
  ComsZMQClientC::~ComsZMQClientC()
  {
    Close();
  }

  //! Close connection
  void ComsZMQClientC::Close()
  {
    m_log->debug("Close called.");

    ComsC::Close();

    if(!m_mutexExitOk.try_lock_for(std::chrono::milliseconds(1000))) {
      m_log->error("Failed to shutdown receiver thread.");
    }
    m_threadRecieve.join();
  }

  //! Is connection ready ?
  bool ComsZMQClientC::IsReady() const
  {
    return true;
  }


  bool ComsZMQClientC::Open(const std::string &portAddr)
  {
    m_terminate = false;


    if(m_terminate)
      return false;

    if(!m_mutexExitOk.try_lock()) {
      m_log->error("Exit lock already locked, multiple threads attempting to open coms ?");
      return false;
    }
    try {
      {
        std::lock_guard<std::mutex> lock(m_accessTx);
        m_client = std::make_shared<zmq::socket_t>(g_zmqContext,ZMQ_PUSH);
        m_client->connect ("tcp://127.0.0.1:7200");
        m_client->setsockopt(ZMQ_SNDTIMEO,500);
      }

      m_threadRecieve = std::move(std::thread { [this]{ RunRecieve(); } });
    } catch(zmq::error_t &err) {
      m_log->error("Caught exception in opening port '%s'  Error: %d '%s' ",portAddr.c_str(),err.num(),err.what());
      return false;
    }
    return true;
  }


  //! Process received packet.

  bool ComsZMQClientC::RunRecieve()
  {
    m_log->debug("Running receiver. ");

    std::shared_ptr<zmq::socket_t> sub = std::make_shared<zmq::socket_t>(g_zmqContext,ZMQ_SUB);
    sub->connect ("tcp://127.0.0.1:7201");
    sub->setsockopt(ZMQ_SUBSCRIBE,0,0);
    sub->setsockopt(ZMQ_RCVTIMEO,500);
    try {
      while(!m_terminate) {
        //bool socket_t::recv(message_t *msg, int flags = 0);
        zmq::message_t msg;
        if(!sub->recv(&msg,0))
          continue;
        //m_log->info("Client got msg.");
        ProcessPacket((uint8_t*)msg.data(),msg.size());
      }
    } catch(zmq::error_t &err) {
      m_log->error("Caught exception in receive thread %d '%s' ",err.num(),err.what());
    }

    m_mutexExitOk.unlock();
    m_log->debug("Exiting receiver. ");

    return true;
  }

  //! Send packet
  void ComsZMQClientC::SendPacket(const uint8_t *buff,int len)
  {
    if(m_terminate)
      return ; // Drop packets if we're terminating.
    try {
      std::lock_guard<std::mutex> lock(m_accessTx);
      zmq::message_t msg (len);
      memcpy (msg.data (), buff, len);
      if(!m_client->send (msg)) {
        m_log->warn("Timeout sending message. ");
      }
    } catch(zmq::error_t &err) {
      m_log->error("Caught exception in transmit thread %d '%s' ",err.num(),err.what());
    }
  }

}
