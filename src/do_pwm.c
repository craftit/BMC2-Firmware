
#include <stdint.h>
#include "hal.h"
#include "pwm.h"
#include <math.h>
#include "chprintf.h"

#define SYSTEM_CORE_CLOCK               168000000


static bool g_pwmThreadRunning = false;

static THD_WORKING_AREA(waThreadPWM, 128);

int g_phaseAngles[12][3];


static int g_drivePhase = 0;

enum PinStateT {
  LOW,
  HIGH,
  PWM,
  NPWM
} ;

struct ComStateC  {
  enum PinStateT m_pinState[6];
  int m_ccer;
  int m_ccmr1;
  int m_ccmr2;
} ;

#if 1
struct ComStateC g_commutationSequence[] = {
    {{  PWM, NPWM, LOW, HIGH, LOW,  LOW }, 0,0,0 }, //  0 AB
    {{  PWM, NPWM, LOW, HIGH, PWM, NPWM }, 0,0,0 }, //  1 AB_CB
    {{  LOW,  LOW, LOW, HIGH, PWM, NPWM }, 0,0,0 }, //  2 CB
    {{  LOW, HIGH, LOW, HIGH, PWM, NPWM }, 0,0,0 }, //  3 CB_CA
    {{  LOW, HIGH, LOW,  LOW, PWM, NPWM }, 0,0,0 }, //  4 CA
    {{  LOW, HIGH, PWM, NPWM, PWM, NPWM }, 0,0,0 }, //  5 CA_BA
    {{  LOW, HIGH, PWM, NPWM, LOW,  LOW }, 0,0,0 }, //  6 BA
    {{  LOW, HIGH, PWM, NPWM, LOW, HIGH }, 0,0,0 }, //  7 BA_BC
    {{  LOW,  LOW, PWM, NPWM, LOW, HIGH }, 0,0,0 }, //  8 BC
    {{  PWM, NPWM, PWM, NPWM, LOW, HIGH }, 0,0,0 }, //  9 BC_AC
    {{  PWM, NPWM, LOW,  LOW, LOW, HIGH }, 0,0,0 }, // 10 AC
    {{  PWM, NPWM, LOW, HIGH, LOW, HIGH }, 0,0,0 }, // 11 AC_AB
    {{  LOW,  LOW, LOW,  LOW, LOW,  LOW }, 0,0,0 }, // 12 STOP
};
#else
struct ComStateC g_commutationSequence[] = {
    {{  PWM, NPWM, LOW, HIGH, LOW,  LOW }, 0,0,0 }, //  0 AB
    {{  LOW,  LOW, LOW, HIGH, PWM, NPWM }, 0,0,0 }, //  1 CB
    {{  LOW, HIGH, LOW,  LOW, PWM, NPWM }, 0,0,0 }, //  2 CA
    {{  LOW, HIGH, PWM, NPWM, LOW,  LOW }, 0,0,0 }, //  3 BA
    {{  LOW,  LOW, PWM, NPWM, LOW, HIGH }, 0,0,0 }, //  4 BC
    {{  PWM, NPWM, LOW,  LOW, LOW, HIGH }, 0,0,0 }, //  5 AC
    {{  LOW,  LOW, LOW,  LOW, LOW,  LOW }, 0,0,0 }, //  6 STOP
};
#endif

/**
 * @brief   Configures and activates the PWM peripheral.
 * @note    Starting a driver that is already in the @p PWM_READY state
 *          disables all the active channels.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */

void PWMUpdateDrive(int phase,int power)
{
#if 1
  if(phase < 0 || phase > 12)
    phase = 12;
  stm32_tim_t *tim = (stm32_tim_t *)TIM1_BASE;
  struct ComStateC *cs = &g_commutationSequence[phase];
  tim->CCER  = cs->m_ccer;
  tim->CCMR1 = cs->m_ccmr1;
  tim->CCMR2 = cs->m_ccmr2;
  tim->CCR[0] = power;
  tim->CCR[1] = power;
  tim->CCR[2] = power;

  tim->EGR = STM32_TIM_EGR_COMG;
#endif
}

static int PWMMode(enum PinStateT ps)
{
  switch(ps)
  {
  case LOW:
  case HIGH:
    return 4;
  case PWM:
  case NPWM:
    return 6;
  }
  return 0;
}

volatile bool g_pwmRun = true;

static THD_FUNCTION(ThreadPWM, arg) {

  (void)arg;
  chRegSetThreadName("pwm");
  //int phase = 0;
  while (g_pwmRun) {
#if 0
    PWMUpdateDrive(phase,200);
    phase += 1;
    if(phase >= 12) phase = 0;
#else
    adcsample_t *vals = ReadADCs();
    float angle = hallToAngle(&vals[9]);
    angle += 5.5;
    if(angle < 0) angle += 24.0;
    if(angle > 24) angle -= 24.0;
    int phase = angle / 2;

    PWMUpdateDrive(phase,400);
#endif
    chThdSleepMicroseconds(5);
    if (!palReadPad(GPIOB, GPIOA_PIN2)) {
      break;
    }
  }
  PWMUpdateDrive(12,0);
  g_pwmThreadRunning = false;
}



int InitPWM(void)
{
  g_drivePhase = 0;

  // Generate the register setup for each phase.

  for(int i = 0;i < 13;i++) {
    struct ComStateC *cs = &g_commutationSequence[i];

    cs->m_ccmr1 =
        STM32_TIM_CCMR1_OC1M(PWMMode(cs->m_pinState[0])) |
        STM32_TIM_CCMR1_OC1PE |
        STM32_TIM_CCMR1_OC2M(PWMMode(cs->m_pinState[2])) |
        STM32_TIM_CCMR1_OC2PE;
    cs->m_ccmr2 =
        STM32_TIM_CCMR2_OC3M(PWMMode(cs->m_pinState[4])) |
        STM32_TIM_CCMR2_OC3PE |
        STM32_TIM_CCMR2_OC4M(6) |
        STM32_TIM_CCMR2_OC4PE;

    int ccer = 0;
    for(int i = 0;i < 6;i++) {
      int nv = 0;
      nv |= STM32_TIM_CCER_CC1E;
      if(i & 1) {
        if((cs->m_pinState[i] == NPWM || cs->m_pinState[i] == HIGH))
          nv |= STM32_TIM_CCER_CC1P;
      } else {
        if((cs->m_pinState[i] == PWM || cs->m_pinState[i] == LOW))
          nv |= STM32_TIM_CCER_CC1P;
      }
      ccer |= nv << (i * 2);
    }
    cs->m_ccer = ccer;
  }


  rccEnableTIM1(FALSE);
  rccResetTIM1();
  //nvicEnableVector(STM32_TIM1_UP_NUMBER, STM32_PWM_TIM1_IRQ_PRIORITY);
  //nvicEnableVector(STM32_TIM1_CC_NUMBER, STM32_PWM_TIM1_IRQ_PRIORITY);
  //pwmp->clock = STM32_TIMCLK2;

  stm32_tim_t *tim = (stm32_tim_t *)TIM1_BASE;

  tim->CCMR1 =
      STM32_TIM_CCMR1_OC1M(6) |
      STM32_TIM_CCMR1_OC1PE |
      STM32_TIM_CCMR1_OC2M(6) |
      STM32_TIM_CCMR1_OC2PE;
  tim->CCMR2 =
      STM32_TIM_CCMR2_OC3M(6) |
      STM32_TIM_CCMR2_OC3PE |
      STM32_TIM_CCMR2_OC4M(6) |
      STM32_TIM_CCMR2_OC4PE;

  uint16_t psc = 0; // (SYSTEM_CORE_CLOCK / 80000000) - 1;
  tim->PSC  = psc;
  tim->ARR  = 2048 - 1; // This should give about 20KHz
  tim->CR2  = 0;
  /* Output enables and polarities setup.*/
  uint16_t ccer = 0;
#if 0
  ccer |= STM32_TIM_CCER_CC1E; // Active high
  //ccer |= STM32_TIM_CCER_CC1P;
  ccer |= STM32_TIM_CCER_CC2E; // Active high
  //ccer |= STM32_TIM_CCER_CC2P;
  ccer |= STM32_TIM_CCER_CC3E;
#endif

  tim->CCER  = ccer;
  tim->EGR   = STM32_TIM_EGR_UG;      /* Update event.                */
  tim->SR    = 0;                     /* Clear pending IRQs.          */
  tim->DIER  = 0;
  tim->BDTR  = STM32_TIM_BDTR_MOE;
  //| STM32_TIM_BDTR_OSSR;

  /* Timer configured and started.*/
  tim->CR1   = STM32_TIM_CR1_ARPE | STM32_TIM_CR1_URS | STM32_TIM_CR1_CEN | STM32_TIM_CR1_CMS(3);

  tim->CCR[0] = 200;
  tim->CCR[1] = 200;
  tim->CCR[2] = 200;

  tim->CR2  = STM32_TIM_CR2_CCPC; // Use the COMG bit to update.

  palSetPad(GPIOC, GPIOC_PIN13); // Wake

  return 0;
}

int PWMRun()
{
  palSetPad(GPIOC, GPIOC_PIN14); // Gate enable
  g_pwmRun = true;
  if(!g_pwmThreadRunning) {
    g_pwmThreadRunning = true;
    chThdCreateStatic(waThreadPWM, sizeof(waThreadPWM), NORMALPRIO, ThreadPWM, NULL);
  }
  return 0;
}

int PWMStop()
{
  g_pwmRun = false;
  palClearPad(GPIOC, GPIOC_PIN14); // Gate disable
  return 0;
}

float hallToAngleDebug(adcsample_t *sensors,int *nearest,int *n0,int *n1,int *n2);

int PWMCal(BaseSequentialStream *chp)
{
  palSetPad(GPIOC, GPIOC_PIN14); // Gate enable


  for(int phase = 0;phase < 12;phase++) {
    PWMUpdateDrive(phase,200);
    chThdSleepMilliseconds(1000);
    adcsample_t *vals = ReadADCs();
    g_phaseAngles[phase][0] = vals[9];
    g_phaseAngles[phase][1] = vals[10];
    g_phaseAngles[phase][2] = vals[11];

    chprintf(chp, "Cal %d : %04d %04d %04d   \r\n",phase,vals[9],vals[10],vals[11]);
  }

  PWMUpdateDrive(12,0);

  while(true) {
    adcsample_t *vals = ReadADCs();
    int nearest,n0,n1,n2;
    float angle = hallToAngleDebug(&vals[9],&nearest,&n0,&n1,&n2);

    int phase = angle * 10;

    chprintf(chp, "Read %4d : %04d %04d %04d  Ne:%d n0:%d n1:%d n2:%d \r\n",phase,vals[9],vals[10],vals[11],nearest,n0,n1,n2);
    chThdSleepMilliseconds(50);

    if (!palReadPad(GPIOB, GPIOA_PIN2)) {
      break;
    }
  }



  palClearPad(GPIOC, GPIOC_PIN14); // Gate disable
  return 0;
}


inline int sqr(int val)
{
   return val * val;
}

inline float mysqrtf(float op1)
{
  if(op1 <= 0.f)
    return 0.f;

   float result;
   __ASM volatile ("vsqrt.f32 %0, %1" : "=w" (result) : "w" (op1) );
   return (result);
}

float hallToAngle(adcsample_t *sensors)
{
  int distTable[12];
  int phase = 0;
  int minDist = sqr(g_phaseAngles[0][0] - sensors[0]) +
                sqr(g_phaseAngles[0][1] - sensors[1]) +
                sqr(g_phaseAngles[0][2] - sensors[2]);
  distTable[0] = minDist;

  for(int i = 1;i < 12;i++) {
    int dist = sqr(g_phaseAngles[i][0] - sensors[0]) +
                  sqr(g_phaseAngles[i][1] - sensors[1]) +
                  sqr(g_phaseAngles[i][2] - sensors[2]);
    distTable[i] = dist;
    if(dist < minDist) {
      phase = i;
      minDist = dist;
    }
  }
  int last = phase - 1;
  if(last < 0) last = 11;
  int next = phase + 1;
  if(last > 11) last = 0;
  int lastDist2 = distTable[last];
  int nextDist2 = distTable[next];
  float nearDist = mysqrtf(minDist);
  float angle = phase * 2.0;
  if(lastDist2 < nextDist2) {
    float lastDist = mysqrtf(lastDist2);
    angle -= nearDist / (lastDist + nearDist);
  } else {
    float nextDist = mysqrtf(nextDist2);
    angle += nearDist / (nextDist + nearDist);
  }
  //if(angle < 0) angle += 24;
  //if(angle > 24) angle -= 24;
  return angle;
}

float hallToAngleDebug(adcsample_t *sensors,int *nearest,int *n0,int *n1,int *n2)
{
  int distTable[12];
  int phase = 0;
  int minDist = sqr(g_phaseAngles[0][0] - sensors[0]) +
                sqr(g_phaseAngles[0][1] - sensors[1]) +
                sqr(g_phaseAngles[0][2] - sensors[2]);
  distTable[0] = minDist;

  for(int i = 1;i < 12;i++) {
    int dist = sqr(g_phaseAngles[i][0] - sensors[0]) +
                  sqr(g_phaseAngles[i][1] - sensors[1]) +
                  sqr(g_phaseAngles[i][2] - sensors[2]);
    distTable[i] = dist;
    if(dist < minDist) {
      phase = i;
      minDist = dist;
    }
  }
  *nearest = phase;
  int last = phase - 1;
  if(last < 0) last = 11;
  int next = phase + 1;
  if(last > 11) last = 0;
  int lastDist2 = distTable[last];
  int nextDist2 = distTable[next];
  *n0 = minDist;
  *n1 = lastDist2;
  *n2 = nextDist2;
  float nearDist = mysqrtf(minDist);
  float angle = phase * 2.0;
  if(lastDist2 < nextDist2) {
    float lastDist = mysqrtf(lastDist2);
    angle -= nearDist / (lastDist + nearDist);
  } else {
    float nextDist = mysqrtf(nextDist2);
    angle += nearDist / (nextDist + nearDist);
  }
  //if(angle < 0) angle += 24;
  //if(angle > 24) angle -= 24;
  return angle;
}




