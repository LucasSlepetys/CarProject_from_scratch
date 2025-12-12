#include "RPM.h"
#include "TIMER.h"

// --- top of file ---
#define N_SLOTS        12
#define TICK_S         4e-6        // prescaler 64 → 4 µs per tick
#define TIMEOUT_MS     200         // a bit more forgiving than 120
#define OUTLIER_BAND   0.35f       // ±35% clamp
#define EMA_ALPHA      0.30f       // recent readings weighted more

// size the "too-fast" gate
#define DT_MIN_TICKS   400         // ~0.8 * dt_min(1200–1500 rpm) //! Needs tunning //! Calculate with expected RPM range



volatile uint16_t last_ticks = 0;
volatile uint16_t period_ticks = 0;
volatile uint8_t  new_period = 0;

ISR(INT0_vect) {

    uint16_t now = TCNT1;
    uint16_t dt  = now - last_ticks;   // handles wrap naturally (unsigned)
    last_ticks   = now;

    // ---- Noise filter (software debounce) ----
    // Ignore edges that arrive "too soon" to be real.
    if (dt > DT_MIN_TICKS) {                    // 100 ticks = 50 µs
        period_ticks = dt;
        new_period   = 1;
    }
}

void RPM_init(void) {

    // Timer1: normal mode, prescaler = 64
    TCCR1A = 0;
    TCCR1B = (1<<CS11) | (1<<CS10);   // clk/64  → 4 µs tick
    TCNT1  = 0; //increases every 0.5 microseconds

    // PD2 = INT0 input, pull-up enabled
    DDRD  &= ~(1 << PD2);
    PORTD |=  (1 << PD2);

    // Falling-edge trigger & enable interrupter
    EICRA &= ~((1 << ISC00) | (1 << ISC01));
    EICRA |=  (1 << ISC01);
    EIMSK |=  (1 << INT0);

    sei();


}

float get_RPM(void) {
    // Shared with ISR:
    extern volatile uint16_t period_ticks;
    extern volatile uint8_t  new_period;

    static float    rpm_filtered = 0.0f; // persistent filtered value
    static uint32_t last_pulse_ms = 0;   // when we last accepted a pulse

    // 1) Copy one new period safely (if available)
    uint16_t dt = 0;
    uint8_t  got = 0;

    cli();
    if (new_period) {
        dt = period_ticks;
        new_period = 0;      // consume exactly one sample
        got = 1;
    }
    sei();

    uint32_t now = millis();

    // 2) If no new pulse, apply timeout-to-zero and return current filtered
    if (!got) {
        if ((now - last_pulse_ms) > TIMEOUT_MS) {
            rpm_filtered = 0.0f;   // declare stop
        }
        return rpm_filtered;
    }

    // 3) We have a new period; update last-pulse timestamp
    last_pulse_ms = now;

    // Guard (shouldn't happen if ISR filters dt)
    if (dt == 0) return rpm_filtered;

    // 4) Convert dt (timer ticks) -> RPM
    //    period T [s] = dt * TICK_S
    //    pulses/sec  = 1 / T
    //    RPM         = (pulses/sec * 60) / N_SLOTS
    double T        = (double)dt * (double)TICK_S;
    double rpm_new  = (60.0 / (double)N_SLOTS) / T;

    // 5) Outlier clamp: limit jumps to ±OUTLIER_BAND around current filtered
    if (rpm_filtered > 1.0f) { // only after we have a baseline
        float lo = rpm_filtered * (1.0f - OUTLIER_BAND);
        float hi = rpm_filtered * (1.0f + OUTLIER_BAND);
        if ((float)rpm_new < lo) rpm_new = lo;
        else if ((float)rpm_new > hi) rpm_new = hi;
    }

    // 6) Exponential Moving Average (more weight on newest sample)
    rpm_filtered += EMA_ALPHA * ((float)rpm_new - rpm_filtered);

    return rpm_filtered;
}


