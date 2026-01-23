#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

/**
 * Pin Definitions for Massage Chair Firmware
 * 
 * This file contains all pin definitions to avoid conflicts
 * with the original hardware definitions.
 */

// Relay 1 - Recline/Incline Control
#define RL1_PWM_PIN PA5
#define RL1_DIR_PIN PA0

// Relay 2 - Forward/Backward Control
#define RL2_PWM_PIN PA4
#define RL2_DIR_PIN PA1

// Relay 3 - Roll Motor Control
#define RL3_PWM_PIN PB1
#define RL3_DIR_PIN PA8

// FET Controls
#define FETT_PWM_PIN PB0  // KNEADING 
#define FETK_PWM_PIN PA7  // COMPRESSION 

// Limit Sensors
#define LMT_UP_PIN PB4
#define LMT_DOWN_PIN PB3

// BLE Module
#define HM10_BREAK_PIN PB9

// Serial Pins
#define DEBUG_TX_PIN PA9
#define DEBUG_RX_PIN PA10
#define BLE_TX_PIN PA2
#define BLE_RX_PIN PA3

#endif // PIN_DEFINITIONS_H
