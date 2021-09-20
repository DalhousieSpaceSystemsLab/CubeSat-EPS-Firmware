#include "clock.h"
#include "driverlib.h"

void clockInit(void){

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


    /* Configuring SysTick to trigger at 1 500 000 (MCLK is 3MHz so this will make
      * it toggle every 0.5s) */
     MAP_SysTick_enableModule();
     MAP_SysTick_setPeriod(MCLK_FREQ); //set period to 10 ms (
     MAP_Interrupt_enableSleepOnIsrExit();
     MAP_SysTick_enableInterrupt();
}
