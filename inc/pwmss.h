#ifndef PLATFORM_PWMSS_H
#define PLATFORM_PWMSS_H

void EPWM1PinMuxSetup();
void EPWM2PinMuxSetup();
void PWMSSTBClkEnable(unsigned int instance);
void PWMSSModuleClkConfig(unsigned int instanceNum);

#endif
