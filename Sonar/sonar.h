#ifndef SONAR_H_
#define SONAR_H

extern void process_SWI(void);
extern void EDMA_interrupt_service(void);
extern void config_EDMA(void);
extern void config_interrutps(void);
extern void SWI_LEDToggle(void);
extern void tsk_led_toggle(void);

	
#endif /*SONAR_H*/
