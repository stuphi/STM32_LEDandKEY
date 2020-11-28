/**
  ******************************************************************************
  * @file           : tm1683.c
  * @brief          : TM1683 LED and Key functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 Philip Stubbs.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  * POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include <main.h>
#include <tm1638.h>


TM1638::TM1638(GPIO_TypeDef* tm_stb_port, uint16_t tm_stb_pin,
               GPIO_TypeDef* tm_clk_port, uint16_t tm_clk_pin,
               GPIO_TypeDef* tm_dio_port, uint16_t tm_dio_pin)
{
  _TM_STB_Port = tm_stb_port;
  _TM_STB_Pin  = tm_stb_pin;
  _TM_CLK_Port = tm_clk_port;
  _TM_CLK_Pin  = tm_clk_pin;
  _TM_DIO_Port = tm_dio_port;
  _TM_DIO_Pin  = tm_dio_pin;
}



void TM1638::set_out()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = _TM_DIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_DeInit(_TM_DIO_Port, _TM_DIO_Pin);
  HAL_GPIO_Init(_TM_DIO_Port, &GPIO_InitStruct);
}

void TM1638::set_in()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = _TM_DIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_DeInit(_TM_DIO_Port, _TM_DIO_Pin);
  HAL_GPIO_Init(_TM_DIO_Port, &GPIO_InitStruct);
}



void TM1638::shift_out(uint8_t val)
{
  for (uint8_t i = 0; i < 8; i++)
    {
      HAL_GPIO_WritePin(_TM_DIO_Port, _TM_DIO_Pin, !!(val & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

      HAL_GPIO_WritePin(_TM_CLK_Port, _TM_CLK_Pin, GPIO_PIN_SET);
      __NOP();
      HAL_GPIO_WritePin(_TM_CLK_Port, _TM_CLK_Pin, GPIO_PIN_RESET);
      __NOP();
    }
}

uint8_t TM1638::shift_in()
{
  uint8_t value = 0;

  for(uint8_t i = 0; i < 8; ++i)
    {
      value |= HAL_GPIO_ReadPin(_TM_DIO_Port, _TM_DIO_Pin) << i;

      HAL_GPIO_WritePin(_TM_CLK_Port, _TM_CLK_Pin, GPIO_PIN_SET);
      __NOP();
      HAL_GPIO_WritePin(_TM_CLK_Port, _TM_CLK_Pin, GPIO_PIN_RESET);
      __NOP();
    }

  return value;
}

void TM1638::send_command(uint8_t data)
{
  TM_CS_SELECT;
  shift_out(data);
  TM_CS_DESELECT;
}

void TM1638::reset()
{
  send_command(TM_WRITE_INC);
  TM_CS_SELECT;
  for(uint16_t i=0; i<16; i++)
    {
      shift_out(0);
    }
  TM_CS_DESELECT;
}

void TM1638::set()
{
  send_command(TM_WRITE_LOC);
  TM_CS_SELECT;
  shift_out(0xC2);
  shift_out(255);
  TM_CS_DESELECT;
}

void TM1638::brightness(uint8_t brightness)
{
  uint8_t  value = 0;
  value = TM_BRIGHT_ADR + (TM_BRIGHT_MASK & brightness);
  send_command(value);
}

void TM1638::display7Seg(uint8_t position, uint8_t value)
{
  send_command(TM_WRITE_LOC);
  TM_CS_SELECT;
  shift_out(TM_SEG_ADR + (position << 1));
  shift_out(value);
  TM_CS_DESELECT;
}

void TM1638::displayASCII(uint8_t position, uint8_t ascii)
{
  display7Seg(position, SevenSeg[ascii - TM_ASCII_OFFSET]);
}

void TM1638::displayASCIIwDot(uint8_t position, uint8_t ascii) {
  // add 128 or 0x080 0b1000000 to turn on decimal point/dot in seven seg
  display7Seg(position, SevenSeg[ascii- TM_ASCII_OFFSET] + TM_DOT_MASK_DEC);
}

void TM1638::displayText(const char *text)
{
  char c, pos;
  pos = 0;
  while ((c = (*text++)) && pos < TM_DISPLAY_SIZE)
    {
      if (*text == '.' && c != '.')
        {
          displayASCIIwDot(pos++, c);
          text++;
        }
      else
        {
          displayASCII(pos++, c);
        }
    }
}

void TM1638::setLED(uint8_t position, uint8_t value)
{
  send_command(TM_WRITE_LOC);
  TM_CS_SELECT;
  shift_out(TM_LEDS_ADR + (position << 1));
  shift_out(value);
  TM_CS_DESELECT;
}

void TM1638::setLEDs(uint8_t ledvalues)
{
  uint8_t led = 0;
  for (uint8_t LEDposition = 0;  LEDposition < 8; LEDposition++)
    {
      if ((ledvalues & (1 << LEDposition)) != 0)
        {
          led = 1;
        }
      else
        {
          led = 0;
        }
      setLED(LEDposition, led);
    }
}

uint8_t TM1638::readButtons()
{
  uint8_t buttons = 0;
  uint8_t v = 0;

  TM_CS_SELECT;
  shift_out(TM_BUTTONS_MODE);
  set_in();
  for (uint8_t i = 0; i < 4; i++)
    {
      v = shift_in() << i;
      buttons |= v;
    }

  set_out();
  TM_CS_DESELECT;
  return buttons;
}
