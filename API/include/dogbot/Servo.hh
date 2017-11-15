#ifndef DOGBOG_SERVO_HEADER
#define DOGBOG_SERVO_HEADER 1

#include "dogbot/SerialComs.hh"
#include <json/json.h>
#include <chrono>

namespace DogBotN {

  class DogBotAPIC;

  //! Motor calibration data.

  class MotorCalibrationC
  {
  public:
    MotorCalibrationC();

    //! Load configuration from JSON
    bool LoadJSON(const Json::Value &conf);

    //! Save calibration as JSON
    Json::Value SaveJSON() const;

    //! Set calibration point
    void SetCal(int place,uint16_t p1,uint16_t p2,uint16_t p3);

    //! Get calibration point
    void GetCal(int place,uint16_t &p1,uint16_t &p2,uint16_t &p3) const;

  protected:
    //std::vector<>
    uint16_t m_hall[18][3];
  };

  //! Information about a single servo.

  class ServoC
  {
  public:
    typedef std::chrono::time_point<std::chrono::steady_clock,std::chrono::duration< double > > TimePointT;

    ServoC(const std::shared_ptr<SerialComsC> &coms,int deviceId);

    //! Access name of device
    const std::string &Name() const
    { return m_name; }

    //! Access the device id.
    int Id() const
    { return m_id; }

    //! Get last reported state of the servo.
    bool GetState(TimePointT &tick,float &position,float &velocity,float &torque) const;

    //! Update torque for the servo.
    bool DemandTorque(float torque);

    //! Demand a position for the servo
    bool DemandPosition(float position,float torqueLimit);

    //! Last reported position
    float Position() const
    { return m_position; }

    //! Last reported torque
    float Torque() const
    { return m_torque; }

    //! Last reported velocity
    float Speed() const
    { return m_speed; }

    //! Last fault code received
    FaultCodeT FaultCode() const
    { return m_faultCode; }

    //! Get the current calibration state.
    MotionCalibrationT CalibrationState() const
    { return m_calibrationState; }

    //! Access the control state.
    ControlStateT ControlState() const
    { return m_controlState; }

    //! Last reported temperature
    float Temperature() const
    { return m_temperature; }

    //! Access supply voltage
    float SupplyVoltage() const
    { return m_supplyVoltage; }

  protected:
    //! Process update
    //! Returns true if state changed
    bool HandlePacketServoReport(const PacketServoReportC &);

    //! Handle an incoming announce message.
    //! Returns true if state changed.
    bool HandlePacketAnnounce(const PacketDeviceIdC &pkt);

    //! Handle parameter update.
    bool HandlePacketReportParam(const PacketParam8ByteC &pkt);

    int32_t m_uid1 = 0;
    int32_t m_uid2 = 0;
    int m_id = -1; // Device id.
    std::string m_name;

    std::shared_ptr<spdlog::logger> m_log = spdlog::get("console");
    std::shared_ptr<SerialComsC> m_coms;

    bool m_online = false;

    FaultCodeT m_faultCode = FC_Ok;
    MotionCalibrationT m_calibrationState = MC_Uncalibrated;
    ControlStateT m_controlState = CS_Fault;
    mutable std::mutex m_mutexState;

    TimePointT m_timeEpoch;
    TimePointT m_timeOfLastReport;

    uint8_t m_lastTimestamp = 0;

    std::chrono::duration<double> m_tickRate; // Default is 100Hz
    unsigned m_tick = 0;
    float m_supplyVoltage = 0;
    float m_position = 0;
    float m_speed = 0;
    float m_torque = 0;
    float m_temperature = 0;



    friend class DogBotAPIC;
  };

}

#endif
