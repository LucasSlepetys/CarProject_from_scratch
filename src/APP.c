// APP.c
#include "APP.h"
#include "NEXTION.h"   // for nextion_send_command, etc.
#include "L298N.h"
#include "TIMER.h"
#include "RPM.h"
#include "MOTOR.h"
#include <string.h>    // if needed

#define N_SLOTS        12 //Could be in systems
#define WHEEL_DIAMETER_M   0.045f
#define PI_F               3.14159265f
#define WHEEL_CIRC_M       (PI_F * WHEEL_DIAMETER_M)  // circumference = pi*D

// -----------------------------------------------------------------------------
//! App state machine (high-level behaviour)
// -----------------------------------------------------------------------------
typedef enum {
    APP_STATE_IDLE = 0,        // waiting on page0
    APP_STATE_FETCH_PARAMS,    // reading n0..n3 from Nextion
    APP_STATE_CALCULATE,       // doing math based on params, PID control
    APP_STATE_RUNNING,         // car running, PID control, updating page1 fields
    APP_STATE_COMPLETED        // car done, waiting for restart or new start
} app_state_t;

// -----------------------------------------------------------------------------
//! Parameter fetch state machine (non-blocking in order to get n0..n3 values from nextion
// -----------------------------------------------------------------------------
typedef enum {
    PARAM_STATE_IDLE = 0, //Not ready to fetch
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

route_params_t g_route_params = {0}; //Initializes all 4 variables to 0

static app_state_t   g_app_state;
static param_state_t g_param_state;

// flags 
static uint8_t g_start_pressed   = 0; //Start button
static uint8_t g_restart_pressed = 0; //Restart button

//! Run Context for when motor is running:
typedef enum {
    SEGMENT_1 = 1,
    SEGMENT_2 = 2
} segment_t;

static segment_t g_seg = SEGMENT_1;

static float g_seg_target_dist_m = 0.0f; 
static float g_seg_target_time_s = 0.0f;
static float g_seg_target_speed_mps = 0.0f;

uint32_t pulses;

static float g_seg_dist_m = 0.0f;      // distance traveled in current segment
static uint32_t g_seg_start_ms = 0;    // Showing time in Nextion
static float current_measured_speed;   //Showing current measured speed in nextion


// ---------- UI helper to display distances and times selected by user in second page in nextion ---------- //Chatgpt for debugging

static void ui_update_page1_title(void)
{
    char cmd[80];
    sprintf(cmd,
            "tTitle.txt=\"S1: %lu m in %lu s  S2: %lu m in %lu s\"",
            (unsigned long)g_route_params.d1,
            (unsigned long)g_route_params.t1,
            (unsigned long)g_route_params.d2,
            (unsigned long)g_route_params.t2);
    nextion_send_command(cmd);
}

// ---------- Param Finete State Machine ----------

static void params_start_fetch(void) {
    g_param_state = PARAM_STATE_REQ_D1;
}

static uint8_t params_is_done(void) {
    return (g_param_state == PARAM_STATE_DONE);
}

//! When App State = APP_STATE_FETCH_PARAMS
// params_update sends requests, waits, and stores (using nextion and other functions) the variables of distances and times
static void params_update(void) {

    switch (g_param_state) {

        case PARAM_STATE_REQ_D1:
            nextion_send_command("get page0.n0.val");
            g_param_state = PARAM_STATE_WAIT_D1;
            break;

        case PARAM_STATE_WAIT_D1:
            break;

        case PARAM_STATE_REQ_T1:
            nextion_send_command("get n1.val");
            g_param_state = PARAM_STATE_WAIT_T1;
            break;

        case PARAM_STATE_WAIT_T1:
            break;

        case PARAM_STATE_REQ_D2:
            nextion_send_command("get n2.val");
            g_param_state = PARAM_STATE_WAIT_D2;
            break;

        case PARAM_STATE_WAIT_D2:
            break;

        case PARAM_STATE_REQ_T2:
            nextion_send_command("get n3.val");
            g_param_state = PARAM_STATE_WAIT_T2;
            break;

        case PARAM_STATE_WAIT_T2:
            break;

        case PARAM_STATE_DONE:
        case PARAM_STATE_IDLE:
        default:
            break;
    }
}

// ! ---------- Called from nextion driver ----------

void app_on_start_pressed(void) {
    g_start_pressed = 1;
}

void app_on_restart_pressed(void) {
    g_restart_pressed = 1;
}

//! After nextion receives a number from get request
// Assigns number to specific varible corresponding to d1, t1, d2, t2 depending on Param_State
void app_on_nextion_number(uint32_t value) {

    switch (g_param_state) {
        case PARAM_STATE_WAIT_D1:
            g_route_params.d1 = value;
            g_param_state = PARAM_STATE_REQ_T1;
            break;

        case PARAM_STATE_WAIT_T1:
            g_route_params.t1 = value;
            g_param_state = PARAM_STATE_REQ_D2;
            break;

        case PARAM_STATE_WAIT_D2:
            g_route_params.d2 = value;
            g_param_state = PARAM_STATE_REQ_T2;
            break;

        case PARAM_STATE_WAIT_T2:
            g_route_params.t2 = value;
            g_param_state = PARAM_STATE_DONE;
            break;

        default:
            // unexpected; ignore
            break;
    }
}

// ---------- App lifecycle ----------

//! Assigns initial states to App_state and Param_state
void app_init(void)
{
    g_app_state   = APP_STATE_IDLE;
    g_param_state = PARAM_STATE_IDLE; //Not ready to fetch
    memset(&g_route_params, 0, sizeof(g_route_params)); //Fancy way to reset the struct back to zero 
}

//! Add State Update
void app_update(void)
{
    switch (g_app_state) {

        // User sees page 0, decides on values  
        //! When "start button" is clicked:
        //* Param State is switched to PARAM_STATE_REQ_D1
        //* App state is switched to APP_STATE_FETCH_PARAMS
        // State is swithced to params_start_fetch : d1, t1, d2, t2 
        case APP_STATE_IDLE:
            if (g_start_pressed) {
                g_start_pressed = 0;
                params_start_fetch();
                g_app_state = APP_STATE_FETCH_PARAMS;
            }
            break;
        
        //! When App state == APP_STATE_FETCH_PARAMS
        // Params_update is called to continously, asyconosly fatch all 4 values 
        case APP_STATE_FETCH_PARAMS:
            params_update();
            if (params_is_done()) {

                //! Param Fetch State machine has finished fatching all values
                g_app_state = APP_STATE_CALCULATE;
            }
            break;

        //! All values have been fetched, ready to start calculations
        // Goes to page 1
        // Updates UI to show desired distance and time
        // Updates status text
        case APP_STATE_CALCULATE:
            nextion_send_command("page page1");
            ui_update_page1_title();
            nextion_send_command("tStatus.txt=\"In progress\"");

            g_seg = SEGMENT_1;
            g_seg_target_dist_m = (float) g_route_params.d1;    
            g_seg_target_time_s = (float)g_route_params.t1;
            
            g_seg_target_speed_mps = g_seg_target_dist_m / g_seg_target_time_s;


            g_seg_dist_m = 0.0f;
            g_seg_start_ms = millis(); // For time display in nextion
            

            RPM_clear_pulses(); //starts counting pulses from zero
            motor_init(); //Starts PI control motor driver

            g_app_state = APP_STATE_RUNNING;
            break;

        //! Car is now running
        // PI control
        // Updates motor speed given octocopler data and desired values
        // Updates fields of nextion while running
        case APP_STATE_RUNNING:

            pulses = RPM_take_pulses_and_clear();    
            g_seg_dist_m += pulses * (WHEEL_CIRC_M / (float)N_SLOTS);

            current_measured_speed = update_motor(g_seg_target_speed_mps);
            // ui_update_runtime_fields(g_seg); //depends on which g_seg we are in for updating
            
            //! Checks if distance of first or second segment has been traveled
            //Switch segments accordingly

            if(g_seg_dist_m >= g_seg_target_dist_m) {

                if (g_seg == SEGMENT_1) {
                    // switch to segment 2
                    g_seg = SEGMENT_2;

                    g_seg_target_dist_m = (float)g_route_params.d2;
                    g_seg_target_time_s = (float)g_route_params.t2;
                    g_seg_target_speed_mps = g_seg_target_dist_m / g_seg_target_time_s;

                    g_seg_dist_m = 0.0f;
                    g_seg_start_ms = millis();

                    RPM_clear_pulses();
                    motor_reset_controller();

                } else {
                    // finished segment 2 -> stop
                    motor_set_speed(0);
                    nextion_send_command("tStatus.txt=\"Completed\"");
                    g_app_state = APP_STATE_COMPLETED;
                }
            }

            break;
        

        // Run has been completed, waits for Restart Button to be pressed
        // When pressed: Returns to intial conditions and page0
        case APP_STATE_COMPLETED:
            if (g_restart_pressed) {
                motor_set_speed(0);
                g_restart_pressed = 0;
                nextion_send_command("page page0");
                g_param_state = PARAM_STATE_IDLE;
                g_app_state   = APP_STATE_IDLE;
            }
            break;
    }
}
