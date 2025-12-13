#ifndef RPM_H
#define RPM_H

#include "system.h"

void RPM_init(void);
float get_RPM(void);

// pulse counting API
uint32_t RPM_get_pulse_count(void);
uint32_t RPM_take_pulses_and_clear(void);
void RPM_clear_pulses(void);


#endif /* RPM_H */