#ifndef PWM_HEADER
#define PWM_HEADER 1

#include "ch.h"

int InitPWM(void);

int PWMRun(void);
int PWMStop(void);
int PWMCal(BaseSequentialStream *chp);
int PWMCalSVM(BaseSequentialStream *chp);
int PWMSVMScan(BaseSequentialStream *chp);

void PWMUpdateDrive(int phase,int power);

extern uint16_t *ReadADCs(void);

float hallToAngle(uint16_t *sensors);

extern binary_semaphore_t g_adcInjectedDataReady;
extern int16_t g_currentADCValue[3];
extern float g_currentZeroOffset[3];
extern uint16_t g_hall[3];
extern int g_adcInjCount;
extern int g_pwmTimeoutCount ;

extern volatile bool g_pwmRun;
extern bool g_pwmThreadRunning;

extern int g_phaseAngles[12][3];

extern float g_current[3];
extern float g_phaseAngle;
extern float g_current_Ibus;

extern float g_demandPhasePosition;
extern float g_demandPhaseVelocity;
extern float g_demandTorque;

extern float g_velocityGain;
extern float g_velocityFilter;
extern float g_positionGain;
extern float g_positionIGain;
extern float g_positionIClamp;

extern float g_torqueLimit;

extern int g_phaseRotationCount;
extern float g_currentPhasePosition;
extern float g_currentPhaseVelocity;

extern float g_motor_p_gain;
extern float g_motor_i_gain;

extern float g_Id;
extern float g_Iq;
extern float g_Ierr_d;
extern float g_Ierr_q;

enum ControlModeT {
  CM_Idle,
  CM_Break,
  CM_Torque,
  CM_Velocity,
  CM_Position
} ;

extern enum ControlModeT g_controlMode;

#endif
