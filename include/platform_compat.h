#ifndef PLATFORM_COMPAT_H
#define PLATFORM_COMPAT_H

#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <AsyncTCP.h>
#include <WiFi.h>
#else
#error "Unsupported Arduino platform"
#endif

inline uint32_t owieChipId() {
#if defined(ARDUINO_ARCH_ESP8266)
  return ESP.getChipId();
#else
  return static_cast<uint32_t>(ESP.getEfuseMac() & 0xFFFFFFFF);
#endif
}

inline void owieSetWifiOutputPower(int powerDbm) {
#if defined(ARDUINO_ARCH_ESP8266)
  WiFi.setOutputPower(powerDbm);
#else
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
#endif
}

#endif  // PLATFORM_COMPAT_H
