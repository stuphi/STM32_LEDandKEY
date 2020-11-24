/*
 * tm1638.h
 *
 *  Created on: Nov 19, 2020
 *      Author: philip
 */

#ifndef INC_TM1638_H_
#define INC_TM1638_H_

#define TM_ACTIVATE 0x8F 			// Start up
#define TM_BUTTONS_MODE 0x42 		// Buttons mode
#define TM_WRITE_LOC 0x44 			// Write to a location
#define TM_WRITE_INC 0x40 			// Incremental write
#define TM_SEG_ADR 0xC0  			// leftmost segment Address C0 C2 C4 C6 C8 CA CC CE
#define TM_LEDS_ADR 0xC1  			// Rightmost LED address C1 C3 C5 C7 C9 CB CD CF
#define TM_BRIGHT_ADR 0x88 			// Brightness address
#define TM_BRIGHT_MASK 0x07 		// Brightness mask
#define TM_DEFAULT_BRIGHTNESS 0x02	//can be 0x00 to 0x07
#define TM_DISPLAY_SIZE 8 			//size of display

#define TM_ASCII_OFFSET 32 			// Ascii table offset to jump over first missing 32 chars
#define TM_HEX_OFFSET   16 			// Ascii table offset to reach number position
#define TM_DOT_MASK_DEC    128 		// 0x80 Mask to  switch on decimal point in seven seg.

#define TM_RED_LED 0x02 // Model 3
#define TM_OFF_LED 0x00

#define TM_CS_SELECT HAL_GPIO_WritePin(TM_CS_GPIO_Port, TM_CS_Pin, GPIO_PIN_RESET)
#define TM_CS_DESELECT HAL_GPIO_WritePin(TM_CS_GPIO_Port, TM_CS_Pin, GPIO_PIN_SET)


void tm_set_out();
void tm_set_in();
void tm_shiftOut(uint8_t val);
uint8_t tm_shiftIn();
void tm_send_command(uint8_t data);
void tm_reset();
void tm_set();
void tm_brightness(uint8_t brightness);
void tm_display7Seg(uint8_t position, uint8_t value);
void tm_displayASCII(uint8_t position, uint8_t ascii);
void tm_displayASCIIwDot(uint8_t position, uint8_t ascii);
void tm_displayText(const char *text);
void tm_setLED(uint8_t position, uint8_t value);
void tm_setLEDs(uint8_t ledvalues);
uint8_t tm_readButtons();

#endif /* INC_TM1638_H_ */
