#include "stm32f10x.h"

// ---------- funciones y defines para el servo ----------
#define SERVO_MIN_PULSE 1000   // 1.0 ms
#define SERVO_MAX_PULSE 2000   // 2.0 ms
#define SERVO_DEFAULT_ANGLE 90 // posición central
#define RX_BUF_SIZE  6   // p.ej. "G+180\n"

volatile char rxBuf[RX_BUF_SIZE];
volatile uint8_t rxCnt = 0;
uint8_t servo_angle = SERVO_DEFAULT_ANGLE;  // 0–180

void servo_init(void);
void servo_move_relative(int8_t delta);
void process_servo_cmd(void);