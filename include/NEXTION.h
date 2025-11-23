#ifndef NEXTION_H
#define NEXTION_H

#include <system.h>
#include <USART.h>
#include <stdint.h>

// -----------------------------------------------------------------------------
// Constants / IDs
// -----------------------------------------------------------------------------
#define FRAME_MAX           64

#define PAGE0_ID            0
#define PAGE1_ID            1
#define START_BUTTON_ID     15
#define RESTART_BUTTON_ID   18

// -----------------------------------------------------------------------------
// App state machine (high-level behaviour)
// -----------------------------------------------------------------------------
typedef enum {
    APP_STATE_IDLE = 0,        // waiting on page0
    APP_STATE_FETCH_PARAMS,    // reading n0..n3 from Nextion
    APP_STATE_CALCULATE,       // doing math based on params, PID control
    APP_STATE_RUNNING,         // car running, PID control, updating page1 fields
    APP_STATE_COMPLETED        // car done, waiting for restart or new start
} app_state_t;

// Values from page0 (route parameters)
typedef struct {
    uint32_t d1;
    uint32_t t1;
    uint32_t d2;
    uint32_t t2;
} route_params_t;

// -----------------------------------------------------------------------------
// Parameter fetch state machine (non-blocking "get n0..n3")
// -----------------------------------------------------------------------------
typedef enum {
    PARAM_STATE_IDLE = 0,
    PARAM_STATE_REQ_D1,
    PARAM_STATE_WAIT_D1,
    PARAM_STATE_REQ_T1,
    PARAM_STATE_WAIT_T1,
    PARAM_STATE_REQ_D2,
    PARAM_STATE_WAIT_D2,
    PARAM_STATE_REQ_T2,
    PARAM_STATE_WAIT_T2,
    PARAM_STATE_DONE
} param_state_t;

// -----------------------------------------------------------------------------
// Global variables (defined in NEXTION.c, visible everywhere via extern)
// -----------------------------------------------------------------------------

// Raw frame buffering (filled by USART RX ISR, consumed by nextion_handle_frame)
extern volatile uint8_t frame_buffer[FRAME_MAX];
extern volatile uint8_t frame_len;
extern volatile uint8_t frame_ready;

// App state + route params + param state
extern volatile app_state_t    g_app_state;
extern volatile route_params_t g_route_params;
extern volatile param_state_t  g_param_state;

// Flags set by touch events
extern volatile uint8_t g_start_pressed;    // Start button pressed on page0
extern volatile uint8_t g_restart_pressed;  // Restart button pressed on page1

// -----------------------------------------------------------------------------
// API functions
// -----------------------------------------------------------------------------

// Called frequently in main loop: copies finished frame and dispatches it
void nextion_handle_frame(void);

// Internal frame handler (called by nextion_handle_frame)
void handle_frame(const uint8_t *buffer, uint8_t len);

// Touch + number callbacks (called from handle_frame)
void ui_handle_touch_event(uint8_t page, uint8_t id, uint8_t event);
void ui_handle_number_response(uint32_t value);

// Parameter-fetch mini state machine
void params_start_fetch(void);
void params_update(void);
uint8_t params_is_done(void);

// High-level app state machine (call this every loop)
void app_update(void);

// Send helpers
void nextion_print_FFs(void);
void nextion_send_command(const char *cmd);

#endif // NEXTION_H
