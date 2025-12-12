// APP.h
#ifndef APP_H
#define APP_H

#include "system.h"

// Values from page0 (route parameters)
typedef struct {
    uint32_t d1;
    uint32_t t1;
    uint32_t d2;
    uint32_t t2;
} route_params_t;

// Global route parameters, defined in app.c
extern route_params_t g_route_params;

// Sets App State and Param State 
void app_init(void);

// Updates App States according to user action
void app_update(void);

// ---- Subroutines used in nextion driver  ----

// called when Start button released on page0
void app_on_start_pressed(void);

// called when Restart button released on page1
void app_on_restart_pressed(void);

// called when Nextion sends back a number (0x71) 
// assigns that number to specific variable depending on Param State
void app_on_nextion_number(uint32_t value);

#endif
