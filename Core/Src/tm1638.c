/*
 * tm1638.c
 *
 *  Created on: Nov 19, 2020
 *      Author: philip
 */

#include <main.h>
#include <tm1638.h>

// font , map of ASCII values/table to 7-segment, offset to position 32.
const unsigned char SevenSeg[] = {
  0x00, /* (space) */
  0x86, /* ! */
  0x22, /* " */
  0x7E, /* # */
  0x6D, /* $ */
  0xD2, /* % */
  0x46, /* & */
  0x20, /* ' */
  0x29, /* ( */
  0x0B, /* ) */
  0x21, /* * */
  0x70, /* + */
  0x10, /* , */
  0x40, /* - */
  0x80, /* . */
  0x52, /* / */
  0x3F, /* 0 */
  0x06, /* 1 */
  0x5B, /* 2 */
  0x4F, /* 3 */
  0x66, /* 4 */
  0x6D, /* 5 */
  0x7D, /* 6 */
  0x07, /* 7 */
  0x7F, /* 8 */
  0x6F, /* 9 */
  0x09, /* : */
  0x0D, /* ; */
  0x61, /* < */
  0x48, /* = */
  0x43, /* > */
  0xD3, /* ? */
  0x5F, /* @ */
  0x77, /* A */
  0x7C, /* B */
  0x39, /* C */
  0x5E, /* D */
  0x79, /* E */
  0x71, /* F */
  0x3D, /* G */
  0x76, /* H */
  0x30, /* I */
  0x1E, /* J */
  0x75, /* K */
  0x38, /* L */
  0x15, /* M */
  0x37, /* N */
  0x3F, /* O */
  0x73, /* P */
  0x6B, /* Q */
  0x33, /* R */
  0x6D, /* S */
  0x78, /* T */
  0x3E, /* U */
  0x3E, /* V */
  0x2A, /* W */
  0x76, /* X */
  0x6E, /* Y */
  0x5B, /* Z */
  0x39, /* [ */
  0x64, /* \ */
  0x0F, /* ] */
  0x23, /* ^ */
  0x08, /* _ */
  0x02, /* ` */
  0x5F, /* a */
  0x7C, /* b */
  0x58, /* c */
  0x5E, /* d */
  0x7B, /* e */
  0x71, /* f */
  0x6F, /* g */
  0x74, /* h */
  0x10, /* i */
  0x0C, /* j */
  0x75, /* k */
  0x30, /* l */
  0x14, /* m */
  0x54, /* n */
  0x5C, /* o */
  0x73, /* p */
  0x67, /* q */
  0x50, /* r */
  0x6D, /* s */
  0x78, /* t */
  0x1C, /* u */
  0x1C, /* v */
  0x14, /* w */
  0x76, /* x */
  0x6E, /* y */
  0x5B, /* z */
  0x46, /* { */
  0x30, /* | */
  0x70, /* } */
  0x01, /* ~ */
};


void tm_set_out()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = TM_IO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_DeInit(TM_IO_GPIO_Port, TM_IO_Pin);
  HAL_GPIO_Init(TM_IO_GPIO_Port, &GPIO_InitStruct);
}

void tm_set_in()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = TM_IO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_DeInit(TM_IO_GPIO_Port, TM_IO_Pin);
  HAL_GPIO_Init(TM_IO_GPIO_Port, &GPIO_InitStruct);
}


//void tm_shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
void tm_shiftOut(uint8_t val)
{
  uint8_t i;

  for (i = 0; i < 8; i++)  {
    HAL_GPIO_WritePin(TM_IO_GPIO_Port, TM_IO_Pin, !!(val & (1 << i)));

    HAL_GPIO_WritePin(TM_CLK_GPIO_Port, TM_CLK_Pin, GPIO_PIN_SET);
    __NOP();
    HAL_GPIO_WritePin(TM_CLK_GPIO_Port, TM_CLK_Pin, GPIO_PIN_RESET);
    __NOP();
  }
}

uint8_t tm_shiftIn()
{
	uint8_t value = 0;

	for(uint8_t i = 0; i < 8; ++i) {
		value |= HAL_GPIO_ReadPin(TM_IO_GPIO_Port, TM_IO_Pin) << i;

	    HAL_GPIO_WritePin(TM_CLK_GPIO_Port, TM_CLK_Pin, GPIO_PIN_SET);
	    __NOP();
	    HAL_GPIO_WritePin(TM_CLK_GPIO_Port, TM_CLK_Pin, GPIO_PIN_RESET);
	    __NOP();
	}

    return value;
}

void tm_send_command(uint8_t data)
{
	TM_CS_SELECT;
	tm_shiftOut(data);
	TM_CS_DESELECT;
}

void tm_reset()
{
  tm_send_command(TM_WRITE_INC);
  TM_CS_SELECT;
  for(int i=0; i<16; i++)
  {
	tm_shiftOut(0);
  }
  TM_CS_DESELECT;
}

void tm_set()
{
  tm_send_command(TM_WRITE_LOC);
  TM_CS_SELECT;
  tm_shiftOut(0xC2);
  tm_shiftOut(255);
  TM_CS_DESELECT;
}

void tm_brightness(uint8_t brightness)
{
  uint8_t  value = 0;
  value = TM_BRIGHT_ADR + (TM_BRIGHT_MASK & brightness);
  tm_send_command(value);
}

void tm_display7Seg(uint8_t position, uint8_t value)
{
  tm_send_command(TM_WRITE_LOC);
  TM_CS_SELECT;
  tm_shiftOut(TM_SEG_ADR + (position << 1));
  tm_shiftOut(value);
  TM_CS_DESELECT;
}

void tm_displayASCII(uint8_t position, uint8_t ascii) {
  tm_display7Seg(position, SevenSeg[ascii - TM_ASCII_OFFSET]);
}

void tm_displayASCIIwDot(uint8_t position, uint8_t ascii) {
  // add 128 or 0x080 0b1000000 to turn on decimal point/dot in seven seg
  tm_display7Seg(position, SevenSeg[ascii- TM_ASCII_OFFSET] + TM_DOT_MASK_DEC);
}

void tm_displayText(const char *text) {
  char c, pos;
  pos = 0;
  while ((c = (*text++)) && pos < TM_DISPLAY_SIZE)  {
    if (*text == '.' && c != '.') {
      tm_displayASCIIwDot(pos++, c);
      text++;
    } else {
      tm_displayASCII(pos++, c);
    }
  }
}

void tm_setLED(uint8_t position, uint8_t value)
{
  tm_send_command(TM_WRITE_LOC);
  TM_CS_SELECT;
  tm_shiftOut(TM_LEDS_ADR + (position << 1));
  tm_shiftOut(value);
  TM_CS_DESELECT;
}

void tm_setLEDs(uint8_t ledvalues)
{
  uint8_t led = 0;
  for (uint8_t LEDposition = 0;  LEDposition < 8; LEDposition++)
  {
    if ((ledvalues & (1 << LEDposition)) != 0)
    {
      led = 1;
    } else
    {
    	led = 0;
    }
    tm_setLED(LEDposition, led);
  }
}

uint8_t tm_readButtons()
{
  uint8_t buttons = 0;
  uint8_t v = 0;

  TM_CS_SELECT;
  tm_shiftOut(TM_BUTTONS_MODE);
  tm_set_in();
  for (uint8_t i = 0; i < 4; i++)
  {

//    if  (_HIGH_FREQ == false)
//        v = shiftIn(_DATA_IO, _CLOCK_IO, LSBFIRST) << i;
//    else
//        v = TM_common.HighFreqshiftin(_DATA_IO, _CLOCK_IO, LSBFIRST) << i;
	v = tm_shiftIn() << i;
    buttons |= v;
  }

  // pinMode(_DATA_IO, OUTPUT);
  tm_set_out();
  TM_CS_DESELECT;
  return buttons;
}
