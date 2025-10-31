#ifndef TIMER_H
#define TIMER_H

#include <system.h>

void MILLIS_INIT(void);
uint32_t millis(void);
void delay_ms(uint32_t ms);

#endif /* TIMER_H */