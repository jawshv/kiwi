#include <ADC.h>
#include <Arduino.h>
#include <IntervalTimer.h>
#include <NBHX711.h>
#include <PacketSerial.h>

#include "../KiwiGPIO.h"
// #include "../Libraries/ADC/ADC.h"

#ifndef KIWI_ESTIMATOR
#define KIWI_ESTIMATOR

#define ADC_BUFFER_LENGTH 128
#define CALIBRATION_INTERVAL_MS 100
#define CALIBRATION_SAMPLES 20

#define HX711_HIST_BUFF 24
#define HX711_GAIN 64

/*
    these constants are derived experimentally to scale the load cell output to the
    appropriate full scale measurement.
*/
#define LC_THRUST_SCALE 7000
#define LC_PROPELLANT_SCALE 3000

class Estimator {

public:
    typedef enum {
        mode_standby,
        mode_sample,
        num_modes,
    } ModeType;

    typedef struct {
        ModeType mode;
        void (Estimator::*method)(void);
    } StateMachineType;

    StateMachineType StateMachine[num_modes] = {
        { mode_standby, &Estimator::sm_standby },
        { mode_sample, &Estimator::sm_sample }
    };

    enum TRANSITIONS {
        num_transitions = 2,
    };

    typedef struct {
        ModeType prev;
        ModeType next;
        void (Estimator::*method)(void);
    } StateMachineTransition;

    StateMachineTransition TransitionTable[num_transitions] = {
        { mode_standby, mode_sample, &Estimator::sm_standby_to_sample },
        { mode_sample, mode_standby, &Estimator::sm_sample_to_standby }
    };

    ModeType mode = mode_standby;

    ADC* adc = new ADC();

    IntervalTimer timer;

    static Estimator* estimator;

    Estimator();

    void init();

    void main();

    void setState(ModeType new_mode);

    void calibrateAll();

private:
    // the estimator produces a real time estimate of thrust, chamber pressure,
    // and injector pressure regardless of the mode, as well as fusing the two to provide a robust
    // estimate of engine performance.
    float chamber_pressure;
    float injector_pressure;
    float thrust;

    // keep track of time elapsed over each loop
    int dt;
    int t_last;

    int calibrationTimer = 0;

    // // write buffer for pressure transmitter ADC data
    size_t _pressure_index_0 = 0;
    size_t _pressure_index_1 = 0;
    uint16_t _pressure_buffer_0[ADC_BUFFER_LENGTH];
    uint16_t _pressure_buffer_1[ADC_BUFFER_LENGTH];

    int sampleInterval = 1;

    NBHX711 lc_propellant(pin_lc_propellant_sda, pin_lc_sck, HX711_HIST_BUFF, HX711_GAIN);
    NBHX711 lc_thrust(pin_lc_thrust_sda, pin_lc_sck, HX711_HIST_BUFF, HX711_GAIN);

    void sm_standby(void);

    void sm_sample(void);

    void sm_standby_to_sample(void);

    void sm_sample_to_standby(void);

    float sample_pressure(uint16_t* buffer, size_t length);

    static void adc_timer_callback(void);

    static void adc_isr(void);
};

#endif