#ifndef CLASSIC_H
#define CLASSIC_H


#define CLASSIC_KEY_RELEASED 0x80
#define ISR_KEYBOARD_INTERRUPT 0X21
#define KEYBOARD_INPUT_PORT 0x60

struct keyboard* classic_init();

#endif