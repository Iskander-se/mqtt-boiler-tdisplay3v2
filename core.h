#pragma once
#include <stdint.h>

struct BoilerStateData {
    float temp_tank;
    float temp_solar;
    uint16_t timerMin;
    uint16_t timerSec;
    char statusStr[10];
    //uint16_t freeTime;
    bool heating;
    bool otaActive;
    bool start;
};

extern BoilerStateData state;

// Состояние кнопок
struct ButtonState {
    bool btn1;
    bool btn2;
};
extern ButtonState buttons;

void coreInit();

void coreTick();
void coreButton();