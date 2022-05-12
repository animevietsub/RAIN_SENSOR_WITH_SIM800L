#ifndef __RELAY_MOTOR_LIBRARY_H__
#define __RELAY_MOTOR_LIBRARY_H__

#define RELAY_CONTROL_PIN 23
#define MOTOR_CONTROL_PIN 22

typedef struct
{
    uint8_t RELAY_EN;
    uint8_t MOTOR_EN;
} rm_control_t;

void RelayMotor_Init(void);
void RelayMotor_MotorOn(rm_control_t *rm_control);
void RelayMotor_MotorOff(rm_control_t *rm_control);
void RelayMotor_RelayOn(rm_control_t *rm_control);
void RelayMotor_RelayOff(rm_control_t *rm_control);
void RelayMotor_Update(rm_control_t *rm_control);

#endif