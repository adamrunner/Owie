#include <Arduino.h>

#include "battery_fuel_gauge.h"
#include "bms_relay.h"
#include "network.h"
#include "packet.h"
#include "settings.h"
#include "task_queue.h"

// UART RX is connected to the *BMS* White line
// UART TX is connected to the *MB* White line
// TX_INPUT_PIN must be soldered to the UART TX
#if defined(ARDUINO_ARCH_ESP32)
#ifndef BMS_UART_RX_PIN
#define BMS_UART_RX_PIN 44
#endif
#ifndef BMS_UART_TX_PIN
#define BMS_UART_TX_PIN 43
#endif
#ifndef TX_INPUT_PIN
#define TX_INPUT_PIN BMS_UART_TX_PIN
#endif
#ifndef TX_INVERSE_OUT_PIN
#define TX_INVERSE_OUT_PIN 6
#endif
#else
#define TX_INPUT_PIN 4
// Connected to the MB B line
#define TX_INVERSE_OUT_PIN 5
#endif

namespace {

// Emulate the RS485 B line by bitbanging the inverse
// of the TX A line.
void IRAM_ATTR txPinRiseInterrupt() { digitalWrite(TX_INVERSE_OUT_PIN, 0); }
void IRAM_ATTR txPinFallInterrupt() { digitalWrite(TX_INVERSE_OUT_PIN, 1); }

#ifdef NO_GLOBAL_INSTANCES
HardwareSerial Serial(0);
#endif
#if defined(ARDUINO_ARCH_ESP32)
HardwareSerial BmsSerial(1);
#else
HardwareSerial &BmsSerial = Serial;
#endif
}  // namespace

BmsRelay *relay;

void bms_setup() {
  relay = new BmsRelay([]() { return BmsSerial.read(); },
                       [](uint8_t b) {
                         // This if statement is what implements locking.
                         if (!Settings->is_locked) {
                           BmsSerial.write(b);
                         }
                       },
                       millis);
#if defined(ARDUINO_ARCH_ESP32)
  BmsSerial.begin(115200, SERIAL_8N1, BMS_UART_RX_PIN, BMS_UART_TX_PIN);
#else
  BmsSerial.begin(115200);
#endif

  // The B line idle is 0
  digitalWrite(TX_INVERSE_OUT_PIN, 0);
  pinMode(TX_INVERSE_OUT_PIN, OUTPUT);

  pinMode(TX_INPUT_PIN, INPUT);
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
#endif

  attachInterrupt(digitalPinToInterrupt(TX_INPUT_PIN), txPinRiseInterrupt,
                  RISING);
  attachInterrupt(digitalPinToInterrupt(TX_INPUT_PIN), txPinFallInterrupt,
                  FALLING);

  relay->addReceivedPacketCallback([](BmsRelay *, Packet *packet) {
#ifdef LED_BUILTIN
    static uint8_t ledState = 0;
    digitalWrite(LED_BUILTIN, ledState);
    ledState = 1 - ledState;
#endif
    streamBMSPacket(packet->start(), packet->len());
  });
  relay->setUnknownDataCallback([](uint8_t b) {
    static std::vector<uint8_t> unknownData = {0};
    if (unknownData.size() > 128) {
      return;
    }
    unknownData.push_back(b);
    streamBMSPacket(&unknownData[0], unknownData.size());
  });

  if (Settings->has_battery_state) {
    FuelGaugeState gaugeState;
    gaugeState.bottomMilliampSeconds =
        Settings->battery_state.bottom_milliamp_seconds;
    gaugeState.currentMilliampSeconds =
        Settings->battery_state.current_milliamp_seconds;
    gaugeState.bottomSoc = Settings->battery_state.bottom_soc;
    gaugeState.topSoc = Settings->battery_state.top_soc;
    relay->getBatteryFuelGauge().restoreState(gaugeState);
  }

  // relay->setPowerOffCallback([]() {
  //   Settings->graceful_shutdown_count++;
  //   const FuelGaugeState &gaugeState = relay->getBatteryFuelGauge().getState();

  //   Settings->has_battery_state = true;
  //   Settings->battery_state.bottom_milliamp_seconds =
  //       gaugeState.bottomMilliampSeconds;
  //   Settings->battery_state.current_milliamp_seconds =
  //       gaugeState.currentMilliampSeconds;
  //   Settings->battery_state.bottom_soc = gaugeState.bottomSoc;
  //   Settings->battery_state.top_soc = gaugeState.topSoc;
  //   saveSettings();
  // });

  relay->setBMSSerialOverride(0xFFABCDEF);

  setupWifi();
  setupWebServer(relay);
  TaskQueue.postRecurringTask([]() { relay->loop(); });
}
