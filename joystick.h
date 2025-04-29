#ifndef JOYSTICK_H
#define JOYSTICK_H

void joystick_task(void *params);

typedef struct {
    uint16_t x;
    uint16_t y;
    bool button;
    char direcao[16];
} JoystickData;

#endif
