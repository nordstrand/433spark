
/**
 * 2-way communication with Nexa devices, using 433 MHz transceiver.
 *
 * Forward Nexa commands from serial port to RF transmitter, and from
 * RF receiver to serial port, using the 433 MHz transmitter/receiver pair
 * on the OOK 433 Plug (or compatible hardware). See RF433Transceiver.h
 * for more info on the physical setup.
 *
 * Author: Johan Herland <johan@herland.net>
 * License: GNU GPL v2 or later
 */

#include "Macros.h"
#include "RF433Transceiver.h"
#include "RingBuffer.h"
#include "PulseParser.h"
#include "NexaCommand.h"



RF433Transceiver rf_port();
RingBuffer<char> rx_bits(1000);
PulseParser pulse_parser(rx_bits);
NexaCommand in_cmd, out_cmd;

int LED = D7; // This one is the built-in tiny one to the right of the USB jack

SYSTEM_MODE(AUTOMATIC);

bool LED_ON = true;

char command[NexaCommand::cmd_str_len] = F("NO RECEIVED");

void setup()
{
    Spark.variable("command", command, STRING);

    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    Serial.begin(9600);
    Serial.println(F("nexa_comm ready:"));
}

void toggleLed() {
    LED_ON = ! LED_ON;
    digitalWrite(LED, LED_ON ? HIGH : LOW);
}

void loop()
{
    bool busy = FALSE;

    while (busy = pulse_parser(rf_port.rx_get_pulse())) {
    }

    if (!rx_bits.r_empty()) {
        //Serial.write((const byte *) rx_bits.r_buf(),
        //     rx_bits.r_buf_len());
        //Serial.write((const byte *) rx_bits.r_wrapped_buf(),
        //     rx_bits.r_wrapped_buf_len());
        if (NexaCommand::from_bit_buffer(in_cmd, rx_bits)) {
            toggleLed();
            Serial.println();
            Serial.print("RX <- ");
            in_cmd.print(Serial);

            in_cmd.to_cmd_str().toCharArray(command, NexaCommand::cmd_str_len + 1);
        }
    }
    else if (!busy && Serial.available() >= NexaCommand::cmd_str_len) {
        char buf[NexaCommand::cmd_str_len];
        size_t buf_read = Serial.readBytesUntil(
            '\n', buf, NexaCommand::cmd_str_len);
        Serial.println();
        Serial.print("Read ");
        Serial.print(buf_read);
        Serial.print(" bytes: ");
        Serial.write((const byte *) buf, buf_read);
        Serial.println();
        if (NexaCommand::from_cmd_str(out_cmd, buf, buf_read)) {
            for (size_t i = 0; i < 5; i++) {
                out_cmd.transmit(rf_port);
                Serial.print("TX -> ");
                out_cmd.print(Serial);
            }
        }
    }
}
