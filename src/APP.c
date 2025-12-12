// APP.c
#include "APP.h"
#include "NEXTION.h"   // for nextion_send_command, etc.
#include <string.h>    // if needed


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
            
            //! Sends information to motor driver to start with correct speed 
            // motor_start_route(&g_route_params);
            g_app_state = APP_STATE_RUNNING;
            break;

        //! Car is now running
        // PI control
        // Updates motor speed given octocopler data and desired values
        // Updates fields of nextion while running
        case APP_STATE_RUNNING:

            // motor_update();
            // ui_update_runtime_fields();

            //TEMP: for now, just mark completed immediately:
            nextion_send_command("tStatus.txt=\"Completed\"");
            g_app_state = APP_STATE_COMPLETED;
            break;
        

        // Run has been completed, waits for Restart Button to be pressed
        // When pressed: Returns to intial conditions and page0
        case APP_STATE_COMPLETED:
            if (g_restart_pressed) {
                g_restart_pressed = 0;
                nextion_send_command("page page0");
                g_param_state = PARAM_STATE_IDLE;
                g_app_state   = APP_STATE_IDLE;
            }
            break;
    }
}
