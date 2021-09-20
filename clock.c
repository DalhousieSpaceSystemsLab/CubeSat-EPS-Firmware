#include "clock.h"
#include "driverlib.h"

/*
 * Master clock set to 20 MHz
 */

void clockInit(void)
{

    //Cores must be turned on one after another.
    PMM_setVcore(PMM_CORE_LEVEL_1);
    PMM_setVcore(PMM_CORE_LEVEL_2);
    PMM_setVcore(PMM_CORE_LEVEL_3);

    UCS_initClockSignal(UCS_MCLK, UCS_XT2CLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_initClockSignal(UCS_MCLK, UCS_REFOCLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_setExternalClockSource(XT1_FREQ, XT2_FREQ);
    UCS_turnOnXT2(UCS_XT2_DRIVE_4MHZ_8MHZ);
    UCS_turnOnLFXT1(UCS_XT1_DRIVE_0, UCS_XCAP_3);
    UCS_initClockSignal(UCS_FLLREF, UCS_XT2CLK_SELECT, UCS_CLOCK_DIVIDER_4);
    UCS_initFLLSettle(MCLK_KHZ, MCLK_FLLREF_RATIO);
    UCS_initClockSignal(UCS_SMCLK, UCS_REFOCLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_initClockSignal(UCS_ACLK, UCS_XT1CLK_SELECT, UCS_CLOCK_DIVIDER_1);
}


void systemTickInit(void){
//Start timer in continuous mode sourced by SMCLK
  Timer_A_initContinuousModeParam initContParam = {0};
  initContParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
  initContParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
  initContParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
  initContParam.timerClear = TIMER_A_DO_CLEAR;
  initContParam.startTimer = false;
  Timer_A_initContinuousMode(TIMER_A1_BASE, &initContParam);

  //Initiaze compare mode
  Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
      TIMER_A_CAPTURECOMPARE_REGISTER_0
      );

  Timer_A_initCompareModeParam initCompParam = {0};
  initCompParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
  initCompParam.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
  initCompParam.compareOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
  initCompParam.compareValue = 50000; //10 Hz
  Timer_A_initCompareMode(TIMER_A1_BASE, &initCompParam);

  Timer_A_startCounter( TIMER_A1_BASE,
          TIMER_A_CONTINUOUS_MODE
              );

}

//******************************************************************************
//
//This is the TIMER1_A3 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER1_A0_VECTOR)))
#endif
void TIMER1_A0_ISR (void)
{
  uint16_t compVal = Timer_A_getCaptureCompareCount(TIMER_A1_BASE,
          TIMER_A_CAPTURECOMPARE_REGISTER_0)
          + COMPARE_VALUE;

  //Toggle P1.0
  GPIO_toggleOutputOnPin(
      GPIO_PORT_P1,
      GPIO_PIN0
      );

  //Add Offset to CCR0
  Timer_A_setCompareValue(TIMER_A1_BASE,
      TIMER_A_CAPTURECOMPARE_REGISTER_0,
      compVal
      );
}
