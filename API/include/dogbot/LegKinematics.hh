#ifndef DOGBOT_LEGKINEMATICS_HEADER
#define DOGBOT_LEGKINEMATICS_HEADER 1

#include <jsoncpp/json/json.h>
#include <cassert>

namespace DogBotN {


  //! This computes the position of the end effector given the end effector

  // l1 = Upper leg length.
  // l2 = Lower leg length

  // Coordinates:
  //  x = sideways
  //  y = forward/back
  //  z = Height above ground
  //
  // Angles:
  //  0 - Roll
  //  1 - Pitch
  //  2 - Knee
  //
  // In the 4 bar linkage code the following variables are used:
  //   theta - Servo position
  //   psi - Joint angle

  class LegKinematicsC
  {
  public:
    //! Default constructor
    LegKinematicsC();

    //! Construct from a json object
    LegKinematicsC(const Json::Value &value);

    //! Create
    LegKinematicsC(float l1,float l2);

    //! Access name of leg.
    const std::string &Name() const
    { return m_name; }

    //! Set name of leg.
    void SetName(const std::string &name)
    { m_name = name; }

    //! Configure from JSON
    bool ConfigureFromJSON(const Json::Value &value);

    //! Get the servo configuration as JSON
    Json::Value ConfigAsJSON() const;

    //! 4 bar linkage angle forward.
    //! Servo to achieved joint angle
    //! theta - Servo position
    //! Returns psi, the knee joint angle.
    //! solution2 - Gives the alternate solution, this shouldn't be required as the robot's possible angles are physically limited.
    float Linkage4BarForward(float theta,bool solution2 = false) const;

    //! 4 bar linkage angle backward,
    //! Joint angle to required servo position.
    //! psi - Required knee joint position
    //! theta - Variable to store the servo position
    //! solution2 - There are two possible solutions, changing this from true to false switches between them.
    // Returns true if angle exists, false if it is unreachable
    bool Linkage4BarBack(float psi,float &theta,bool solution2 = false) const;

    //! See: https://synthetica.eng.uci.edu/mechanicaldesign101/McCarthyNotes-2.pdf
    bool Linkage4Bar(float theta,float a,float b,float g,float h,float &psi,bool solution2 = false) const;

    //! Compute the speed ratio at the given input angle. This can also be used to convert torques
    //! theta - Servo position
    //! psi - Joint angle
    float LinkageSpeedRatio(float theta,float psi) const;

    //! Inverse kinematics for the leg using a virtual joint for the knee
    //! Compute joint angles needed to get to a 3d position in a leg coordinate system
    //! Return true if position is reachable
    bool InverseVirtual(float at[3],float (&angles)[3]) const;

    //! Forward kinematics for the leg using a virtual joint for the knee
    //! Compute the position of the foot relative to the top of the leg from the joint angles.
    bool ForwardVirtual(float angles[3],float (&at)[3]) const;

    //! Inverse kinematics for the leg
    //! Compute joint angles needed to get to a 3d position in a leg coordinate system
    //! Return true if position is reachable
    bool InverseDirect(float at[3],float (&angles)[3]) const;

    //! Forward kinematics for the leg
    //! Compute the position of the foot relative to the top of the leg from the joint angles.
    bool ForwardDirect(float angles[3],float (&at)[3]) const;

    //! Use alternate solution ?
    bool UseAlternateSolution() const
    { return m_alternateSolution; }

    // ! Access joint directions
    float JointDirection(int jnt) const
    {
      assert(jnt >= 0 && jnt < 3);
      return m_jointDirections[jnt];
    }

    //! Length of upper leg
    float LengthUpperLeg() const
    { return m_l1; }

    //! Length of lower leg
    float LengthLowerLeg() const
    { return m_l2; }

    //! Offset from roll axis of rotation
    float LengthZDrop() const
    { return m_zoff; }

    //! Length of centre of hip cam
    float LengthLinkHip() const
    { return m_linkA; }

    //! Length of centre of knee cam
    float LengthLinkKnee() const
    { return m_linkB; }

    //! Length of push rod
    float LengthPushRod() const
    { return m_linkH; }

    //! Distance bellow the pivot of the centre of the foot
    float LengthFootDrop() const
    { return m_footDrop; }

    //! Radius of foot sphere
    float FootSphereRadius() const
    { return m_footSphereRadius; }

    //! Leg origin.
    float LegOrigin(int coordinate) const
    { return m_legOrigin[coordinate]; }

  protected:
    std::string m_name; // Leg name

    // These are used to compute the leg positions, though they don't really belong here.
    float m_bodyWidth = 0.304;
    float m_bodyLength = 0.556;

    float m_legOrigin[3] = {0,0,0};
    float m_jointDirections[3] = { 1.0, 1.0, 1.0 };

    float m_l1 = 0.361; // Upper leg length
    float m_l2 = 0.29;  // Lower leg length
    float m_zoff = 0.08;// Vertical offset from central axis

    float m_linkA = 0.032; // Hip CAD: 0.032
    float m_linkB = 0.04;  // Knee CAD: 0.04
    float m_linkH = 0.363; // Push rod 0.363

    float m_footDrop = 0.025; // Distance of centre of foot bellow
    float m_footSphereRadius = 0.05; // Radius of foot sphere

    bool m_alternateSolution = false;

  };

}

#endif
