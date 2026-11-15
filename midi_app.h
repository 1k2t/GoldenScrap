#pragma once

#include "tusb_config.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define MIDI_DATA_RATE_BPS (31250)

typedef struct {
    uart_inst_t *uart;
    uint pin_tx, pin_rx;
    uint8_t itf;  // USB-MIDI interface index (0 = port A, 1 = port B)
    uint8_t buffer[CFG_TUD_MIDI_TX_BUFSIZE / 4 * 3];
    uint32_t buf_pos;
    uint32_t buf_valid;
} MidiApp;

MidiApp *MidiApp_init(MidiApp *midi, uart_inst_t *uart, uint pin_tx, uint pin_rx, uint8_t itf);
void MidiApp_task(MidiApp *midi);
