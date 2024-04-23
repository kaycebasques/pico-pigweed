# Kudzu PCB Changelog

## Rev 1

### Changes

- Updated board outline and keepouts to match CAD model.
- Fixed reversed pinout on `FPC-Connector_1x40-1MP_P0.50mm_Horizontal`.
- Fixed pinout on charge rate FET (`Q3`).
- Swapped `TX` and `GND` on UART header so that `GND` is on pin 1.
- Updated board to reflect above changes.
- Corrected `BOOT` and `RESET` button labels.
- Increased size of labels.
- Added fiducials to board.
- Added DigiKey part numbers to BOM for passives.

## Rev 0

### Changes

- Some routing cleanup.
- Move thermal vias under QFN packages to avoid solder wicking.
- Remove overlapping silk on `CONN HEADER SMD 2POS 1.25MM.kicad_mod `.
- Fixed swapped DISP_SDO & DISP_SDI.

## 0-rc2

### Changes

- Length match QSPI flash traces.
- Impedance control and length match USB traces.
- Length match GBA Link traces.
- Keep GBA Link and QSPI traces on top layer (directly above ground plane) as
  much as possible.

## 0-rc1

### Changes

- Added 2.2uF and 100nF decoupling to flash chip.
- Changed both RP2040 and IO expander pinout to aid in routing.
- Remove `RESET_N` test point
- Confirmed battery connector pinout.
- Change pinout of display backlight driver to aid in routing.
- Select right angle LEDs for indicators.
- Board routed.



## WIP7

### Changes

- Pruned unused parts from `kudzu-parts.kicad_sym`.
- Updated all parts in `kudzu-parts.kicad_sym` to pass `klc-check/check_symbol.py`.
- Switched to `PI4IOE5V6416` IO expander which has programmable pull-up.
- Removed pull-ups on `BOARD_ID*`, `BUTTON_*`, `TOUCH_RESET`, and `DISP_RESET`
  in favor of internal pull-ups in the IO expander.
- Added pull-up to `RESET_N`.
- Removed test points from `QSPI_*`.
- Switched to QFN version of `MCP73831`.
- Fixed missing pin number on `MAX17048` `QSTRT`.
- Removed pull-up on 3.3V regulator's `EN` pin and tied directly to `VIN`.
- Added extra decoupling caps to rp2040's 3.3V and 1.1V lines per design guide.
- Added note to add axis diagram to silk screen on board.
- Miscellaneous visual cleanup.
- Added FET on conduction path for system power.
- Added circuit to charge at 250mA when system is on and 500mA when system is off.
- Assigned all RP2040 IOs.
- Created and switched to dedicated symbols for DPAD, A/B, and SELECT/START.
- Switched display connector symbol for one with a mounting pin.
- Set test point values to net names.
- Selected LEDs and current limiting resistor values.
- Added closed solder jumpers to GBA Link signals.
- Changed solder jumper on GBA VCC pin to open.
- Changed LCD decoupling to mach reference circuit.
- Connected DISP_RD (J9 Pin 12) to +3V3.
- Connected DB0-15 (J9 Pins 17-32) to GND.
- Updated 12MHz Crystal (Y1) to correct symbol and foot print.
