/*******************************************************
*                                                      *
*   Criado em: Fri 20/Jun/2018,  02:34 hs              *
*   Instituicao: Universidade Federal do Ceará         *
*   Autor: Robert Cabral                               *
*   email: robertcabral@alu.ufc.br                     *
*                                                      *
********************************************************/

#include	"hw_types.h"
#include	"soc_AM335x.h"
#include "pwmss.h"
#include "ehrpwm.h"
/*****************************************************************************
**                INTERNAL MACRO DEFINITIONS
*****************************************************************************/
#define TIME													200000
#define TOGGLE          										(0x01u)

#define CM_PER_GPIO1											0xAC
#define CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE   				(0x2u)
#define CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK   			(1 << 18)

#define CM_conf_gpmc_ben1      	 								0x0878
#define CM_conf_gpmc_a5         								0x0854

#define GPIO_OE                 								0x134
#define GPIO_CLEARDATAOUT       								0x190
#define GPIO_SETDATAOUT         								0x194

#define CLOCK_DIV_VAL                 (10)
#define SOC_EHRPWM_2_MODULE_FREQ      (100000000)
#define SOC_EHRPWM_1_MODULE_FREQ      (100000000)

enum duty {
  PWM_DUTY_12_5 = 0,
  PWM_DUTY_25,
  PWM_DUTY_37_5,
  PWM_DUTY_50,
  PWM_DUTY_62_5,
  PWM_DUTY_75,  
  PWM_DUTY_87_5
};


unsigned int flagBlink;

static void delay();
static void ledInit();
static void ledToggle();


void configPWM1(){
    PWMSSModuleClkConfig(1);
    EPWM1PinMuxSetup();
    PWMSSTBClkEnable(1);

    EHRPWMTimebaseClkConfig(SOC_EPWM_1_REGS,
                            SOC_EHRPWM_1_MODULE_FREQ/CLOCK_DIV_VAL,
                            SOC_EHRPWM_1_MODULE_FREQ);

   /* Configure the period of the output waveform */
    EHRPWMPWMOpFreqSet(SOC_EPWM_1_REGS,
                       SOC_EHRPWM_1_MODULE_FREQ/CLOCK_DIV_VAL,
                       (unsigned int)(SOC_EHRPWM_1_MODULE_FREQ/CLOCK_DIV_VAL)/0xFF,
                       (unsigned int)EHRPWM_COUNT_UP,
                       (bool)EHRPWM_SHADOW_WRITE_DISABLE);

    /* Disable synchronization*/
    EHRPWMTimebaseSyncDisable(SOC_EPWM_1_REGS);

    /* Disable syncout*/
    EHRPWMSyncOutModeSet(SOC_EPWM_1_REGS, EHRPWM_SYNCOUT_DISABLE);

    /* Configure the emulation behaviour*/
    EHRPWMTBEmulationModeSet(SOC_EPWM_1_REGS, EHRPWM_STOP_AFTER_NEXT_TB_INCREMENT);

    /* Configure Counter compare cub-module */
    /* Load Compare A value */
    EHRPWMLoadCMPA(SOC_EPWM_1_REGS,
                   50,
                   (bool)EHRPWM_SHADOW_WRITE_DISABLE,
                   (unsigned int)EHRPWM_COMPA_NO_LOAD,
                   (bool)EHRPWM_CMPCTL_OVERWR_SH_FL);

    /* Load Compare B value */
    EHRPWMLoadCMPB(SOC_EPWM_1_REGS,
                   200,
                   (bool)EHRPWM_SHADOW_WRITE_DISABLE,
                   (unsigned int) EHRPWM_COMPB_NO_LOAD,
                   (bool)EHRPWM_CMPCTL_OVERWR_SH_FL);

    /* Configure Action qualifier */
    /* Toggle when CTR = CMPA */
    EHRPWMConfigureAQActionOnB(SOC_EPWM_1_REGS,
                               EHRPWM_AQCTLB_ZRO_DONOTHING,
                               EHRPWM_AQCTLB_PRD_DONOTHING,
                               EHRPWM_AQCTLB_CAU_EPWMXBTOGGLE,
                               EHRPWM_AQCTLB_CAD_DONOTHING,
                               EHRPWM_AQCTLB_CBU_DONOTHING,
                               EHRPWM_AQCTLB_CBD_DONOTHING,
                               EHRPWM_AQSFRC_ACTSFB_DONOTHING);

    /* Bypass dead band sub-module */
    EHRPWMDBOutput(SOC_EPWM_1_REGS, EHRPWM_DBCTL_OUT_MODE_BYPASS);

    /* Disable Chopper sub-module */
    //EHRPWMChopperDisable(SOC_EPWM_2_REGS);

    /* Disable trip events */
    EHRPWMTZTripEventDisable(SOC_EPWM_1_REGS,(bool)EHRPWM_TZ_ONESHOT);
    EHRPWMTZTripEventDisable(SOC_EPWM_1_REGS,(bool)EHRPWM_TZ_CYCLEBYCYCLE);

    /* Event trigger */
    /* Generate interrupt every 3rd occurance of the event */
    EHRPWMETIntPrescale(SOC_EPWM_1_REGS, EHRPWM_ETPS_INTPRD_THIRDEVENT);
    /* Generate event when CTR = CMPB */
    EHRPWMETIntSourceSelect(SOC_EPWM_1_REGS, EHRPWM_ETSEL_INTSEL_TBCTREQUCMPBINC);

    /* Disable High resolution capability */
    EHRPWMHRDisable(SOC_EPWM_1_REGS);

    EHRPWMChopperEnable(SOC_EPWM_1_REGS);
}

void configPWM2(){
    PWMSSModuleClkConfig(2);
    EPWM2PinMuxSetup();
    PWMSSTBClkEnable(2);

    EHRPWMTimebaseClkConfig(SOC_EPWM_2_REGS,
                            SOC_EHRPWM_2_MODULE_FREQ/CLOCK_DIV_VAL,
                            SOC_EHRPWM_2_MODULE_FREQ);

   /* Configure the period of the output waveform */
    EHRPWMPWMOpFreqSet(SOC_EPWM_2_REGS,
                       SOC_EHRPWM_2_MODULE_FREQ/CLOCK_DIV_VAL,
                       (unsigned int)(SOC_EHRPWM_2_MODULE_FREQ/CLOCK_DIV_VAL)/0xFF,
                       (unsigned int)EHRPWM_COUNT_UP,
                       (bool)EHRPWM_SHADOW_WRITE_DISABLE);

    /* Disable synchronization*/
    EHRPWMTimebaseSyncDisable(SOC_EPWM_2_REGS);

    /* Disable syncout*/
    EHRPWMSyncOutModeSet(SOC_EPWM_2_REGS, EHRPWM_SYNCOUT_DISABLE);

    /* Configure the emulation behaviour*/
    EHRPWMTBEmulationModeSet(SOC_EPWM_2_REGS, EHRPWM_STOP_AFTER_NEXT_TB_INCREMENT);

    /* Configure Counter compare cub-module */
    /* Load Compare A value */
    EHRPWMLoadCMPA(SOC_EPWM_2_REGS,
                   200,
                   (bool)EHRPWM_SHADOW_WRITE_DISABLE,
                   (unsigned int)EHRPWM_COMPA_NO_LOAD,
                   (bool)EHRPWM_CMPCTL_OVERWR_SH_FL);

    /* Load Compare B value */
    EHRPWMLoadCMPB(SOC_EPWM_2_REGS,
                   200,
                   (bool)EHRPWM_SHADOW_WRITE_DISABLE,
                   (unsigned int) EHRPWM_COMPB_NO_LOAD,
                   (bool)EHRPWM_CMPCTL_OVERWR_SH_FL);

    /* Configure Action qualifier */
    /* Toggle when CTR = CMPA */
    EHRPWMConfigureAQActionOnB(SOC_EPWM_2_REGS,
                               EHRPWM_AQCTLB_ZRO_DONOTHING,
                               EHRPWM_AQCTLB_PRD_DONOTHING,
                               EHRPWM_AQCTLB_CAU_EPWMXBTOGGLE,
                               EHRPWM_AQCTLB_CAD_DONOTHING,
                               EHRPWM_AQCTLB_CBU_DONOTHING,
                               EHRPWM_AQCTLB_CBD_DONOTHING,
                               EHRPWM_AQSFRC_ACTSFB_DONOTHING);

    EHRPWMConfigureAQActionOnA(SOC_EPWM_2_REGS,
                               EHRPWM_AQCTLB_ZRO_DONOTHING,
                               EHRPWM_AQCTLB_PRD_DONOTHING,
                               EHRPWM_AQCTLB_CAU_EPWMXBTOGGLE,
                               EHRPWM_AQCTLB_CAD_DONOTHING,
                               EHRPWM_AQCTLB_CBU_DONOTHING,
                               EHRPWM_AQCTLB_CBD_DONOTHING,
                               EHRPWM_AQSFRC_ACTSFB_DONOTHING);

    /* Bypass dead band sub-module */
    EHRPWMDBOutput(SOC_EPWM_2_REGS, EHRPWM_DBCTL_OUT_MODE_BYPASS);

    /* Disable Chopper sub-module */
    //EHRPWMChopperDisable(SOC_EPWM_2_REGS);

    /* Disable trip events */
    EHRPWMTZTripEventDisable(SOC_EPWM_2_REGS,(bool)EHRPWM_TZ_ONESHOT);
    EHRPWMTZTripEventDisable(SOC_EPWM_2_REGS,(bool)EHRPWM_TZ_CYCLEBYCYCLE);

    /* Event trigger */
    /* Generate interrupt every 3rd occurance of the event */
    EHRPWMETIntPrescale(SOC_EPWM_2_REGS, EHRPWM_ETPS_INTPRD_THIRDEVENT);
    /* Generate event when CTR = CMPB */
    EHRPWMETIntSourceSelect(SOC_EPWM_2_REGS, EHRPWM_ETSEL_INTSEL_TBCTREQUCMPBINC);

    /* Disable High resolution capability */
    EHRPWMHRDisable(SOC_EPWM_2_REGS);

    EHRPWMChopperEnable(SOC_EPWM_2_REGS);
}

int main(void){
    configPWM1();
    configPWM2();
	
    int a = 0, b = 7;

    while(1){
      EHRPWMConfigureChopperDuty(SOC_EPWM_2_REGS, a);
      a = (a + 1) % 7;
      EHRPWMConfigureChopperDuty(SOC_EPWM_1_REGS, b);
      b == 0 ? b = 7 : b--;
      delay();
    }

	return(0);
}


static void delay(){
	volatile unsigned int ra;
	for(ra = 0; ra < TIME; ra ++);
}