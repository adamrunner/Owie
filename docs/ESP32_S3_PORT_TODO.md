# ESP32-S3 Port TODO

## Current checkpoint

- Added a `seeed_xiao_esp32s3` PlatformIO environment.
- Added ESP8266/ESP32 compatibility helpers.
- Ported ESP32 settings storage to `Preferences`.
- Moved ESP32 BMS UART traffic to `HardwareSerial(1)`.
- Kept the original interrupt-based inverse-TX relay path available for now.
- Added a WireViz wiring diagram for a dual 3.3V SP3485 RS485-transceiver install.
- Verified native tests plus ESP8266 and XIAO ESP32-S3 builds.

## Next steps

1. Decide between minimal inverse-TX wiring and the dual-transceiver wiring.
   - Minimal wiring saves space but needs logic-analyzer validation before board install.
   - Dual transceivers are electrically cleaner but may be hard to package in a Pint controller box.

2. Bench validate UART pin choices on the XIAO ESP32-S3.
   - Confirm GPIO44 / D7 works as BMS UART RX.
   - Confirm GPIO43 / D6 works as controller UART TX.
   - If using inverse-TX mode, confirm GPIO6 / D5 can mirror inverted TX cleanly.

3. If using RS485 transceivers, add a firmware mode for them.
   - Add `OWIE_USE_RS485_TRANSCEIVER`.
   - Skip `TX_INPUT_PIN` interrupt setup in transceiver mode.
   - Keep UART relay logic unchanged.

4. Measure physical fit before soldering.
   - Measure available length, width, and height inside the Pint controller box.
   - Compare XIAO plus two breakout boards against the available volume.
   - Check wire bend radius and strain relief, not just board footprint.

5. Validate BMS receive-only behavior on the bench.
   - Power the XIAO safely from USB first.
   - Capture UART RX with a logic analyzer.
   - Confirm packets decode at 115200 baud and start with `FF 55 AA`.

6. Validate full inline relay on the bench.
   - Confirm forwarded UART TX packets match received packets, except intentional Owie mutations.
   - Confirm lock mode suppresses controller-side traffic.
   - Confirm OTA/recovery still work after settings persistence on ESP32.

7. Address security before real-world use.
   - Add authentication for settings, lock/unlock, raw websocket data, and OTA.
   - Consider disabling station-mode control routes unless explicitly enabled.

8. Update docs after hardware is chosen.
   - Finalize the WireViz diagram for the actual board/module choice.
   - Add pin mapping, build target, flashing, and bench-test instructions.
