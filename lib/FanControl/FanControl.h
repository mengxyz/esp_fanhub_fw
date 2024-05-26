#if !defined(FAN_CONTROL_H)
#define FAN_CONTROL_H

#include <Arduino.h>

#define PIN_TAC_1 41
#define PIN_TAC_2 40
#define PIN_TAC_3 39
#define PIN_TAC_4 38
#define PIN_TAC_5 42

#define PIN_PWM_1 6
#define PIN_PWM_2 5
#define PIN_PWM_3 4
#define PIN_PWM_4 20
#define PIN_PWM_5 19

#define PWM_CH_1 0
#define PWM_CH_2 1
#define PWM_CH_3 2
#define PWM_CH_4 3
#define PWM_CH_5 4

#define PWM_FREQ 25000
#define PWM_RES 8
#define PWM_DEFAULT_DUTY 127

#define BTN_UTIL_PIN_1 15 // PWM +
#define BTN_UTIL_PIN_2 16 // PWM -

class FanControl
{
    static void IRAM_ATTR TAC_1_ISR();
    static void IRAM_ATTR TAC_2_ISR();
    static void IRAM_ATTR TAC_3_ISR();
    static void IRAM_ATTR TAC_4_ISR();
    static void IRAM_ATTR TAC_5_ISR();
    static void IRAM_ATTR BTN_UTIL_1_ISR();
    static void IRAM_ATTR BTN_UTIL_2_ISR();
    static FanControl* instance;
private:
    int pwm_freq;
    int pwm_res;
    uint8_t pwm_freqs[5] = {0, 0, 0, 0, 0};

    uint8_t pwm_duties[5] = {PWM_DEFAULT_DUTY, PWM_DEFAULT_DUTY, PWM_DEFAULT_DUTY, PWM_DEFAULT_DUTY, PWM_DEFAULT_DUTY};
    void initPwmGenerator();
    void initTachometer();

public:
    FanControl(int pwm_freq = PWM_FREQ, int pwm_res = PWM_RES);
    void begin();
    void beginBtnUtils();
    void setAllDuty(uint8_t duty);
};

#endif // FAN_CONTROL_H