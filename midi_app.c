/*
 * USB to serial MIDI adapter
 *
 * Adapted from the TinyUSB midi_test example by Rene Stange
 */

/*
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"
#include "dac.h"
#include "uac_cap.h"
#include "midi_app.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

// #define IGNORE_MIDI_CC

/*------------- MAIN -------------*/
/*
  int main(void)
  {
    board_init();
    // stdio_init_all();
    tusb_init();

    //printf("Hello, world!\n");
    // Initialise UART 0
    uart_init(uart0, 31250);
    
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    // Start DAC format converter
    audio_init();
    uac_cap_init();

    while (1)
    {
      tud_task(); // tinyusb device task
      led_task();
      midi_task();
      uac_cap_task();
    }

    return 0;
  }
*/

MidiApp *MidiApp_init(MidiApp *this, uart_inst_t *uart, uint pin_tx, uint pin_rx, uint8_t itf){
  this->uart = uart;
  this->pin_rx = pin_rx;
  this->pin_tx = pin_tx;
  this->itf = itf;
  this->buf_pos = 0;
  this->buf_valid = 0;
  uart_init(this->uart, MIDI_DATA_RATE_BPS);
  gpio_set_function(this->pin_rx, GPIO_FUNC_UART);
  gpio_set_function(this->pin_tx, GPIO_FUNC_UART);
  // Idle-high is the MIDI rest state; pull the RX pin up so that an
  // unconnected RX (port B has no return line on the module) does not
  // pick up noise and generate phantom input.
  gpio_pull_up(this->pin_rx);
  return this;
}
//--------------------------------------------------------------------+
// UART Helper
//--------------------------------------------------------------------+

static size_t
__time_critical_func(uart_read)(uart_inst_t *uart, uint8_t *dst, size_t maxlen)
{
  size_t result = 0;

  while (maxlen && uart_is_readable(uart))
  {
    *dst++ = uart_getc(uart);

    result++;
    maxlen--;
  }

  return result;
}

//--------------------------------------------------------------------+
// MIDI Task
//--------------------------------------------------------------------+

void midi_task(void)
{
  if (!tud_midi_mounted())
  {
    return;
  }

  // Handle USB to UART direction
  uint8_t packet[4];
  if (tud_midi_available() && uart_is_writable(uart0))
  {
    tud_midi_packet_read(packet);

    static const size_t cin_to_length[] =
        {0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1};

    uint8_t cid = packet[0] & 0xF;
#ifdef IGNORE_MIDI_CC
    if (cid != 0x0B && cid != 0x0C)
#endif
    {
      size_t length = cin_to_length[cid];
      if (length)
      {
        uart_write_blocking(uart0, packet + 1, length);
      }
    }
  }
  else
  {
    ;
  }

  // Handle UART to USB direction
  static uint8_t buffer[CFG_TUD_MIDI_TX_BUFSIZE / 4 * 3];
  static uint32_t buf_pos = 0;
  static uint32_t buf_valid = 0;

  size_t length = buf_valid - buf_pos;
  if (length)
  {
    buf_pos += tud_midi_stream_write(0, buffer + buf_pos, length);
    if (buf_pos < buf_valid)
    {
      return;
    }
  }

  buf_valid = uart_read(uart0, buffer, sizeof buffer);
  if (buf_valid)
  {
    buf_pos = tud_midi_stream_write(0, buffer, buf_valid);

  }
  else
  {
    buf_pos = 0;
  }
}

void MidiApp_task(MidiApp *m)
{
  if (!tud_midi_n_mounted(m->itf))
  {
    return;
  }

  // Handle USB to UART direction. Each USB-MIDI function has its own
  // receive FIFO in the TinyUSB driver, so this instance only ever sees
  // packets that arrived on its own interface; the cable number inside
  // each packet is always 0 (single-cable functions).
  static const size_t cin_to_length[] =
      {0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1};

  uint8_t packet[4];
  while (tud_midi_n_available(m->itf, 0))
  {
    tud_midi_n_packet_read(m->itf, packet);

    uint8_t cid = packet[0] & 0xF;
#ifdef IGNORE_MIDI_CC
    if (cid != 0x0B && cid != 0x0C)
#endif
    {
      size_t length = cin_to_length[cid];
      if (length)
      {
        uart_write_blocking(m->uart, packet + 1, length);
      }
    }
  }

  // Handle UART to USB direction

  size_t length = m->buf_valid - m->buf_pos;
  if (length)
  {
    m->buf_pos += tud_midi_n_stream_write(m->itf, 0, m->buffer + m->buf_pos, length);
    if (m->buf_pos < m->buf_valid)
    {
      return;
    }
  }

  m->buf_valid = uart_read(m->uart, m->buffer, sizeof(m->buffer));
  if (m->buf_valid)
  {
    m->buf_pos = tud_midi_n_stream_write(m->itf, 0, m->buffer, m->buf_valid);

  }
  else
  {
    m->buf_pos = 0;
  }
}
