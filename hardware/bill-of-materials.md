# Bill of Materials — Solar BEMS (As-Built)

This is the **final, as-built** component list for the implemented Solar BEMS, matching the
firmware in [`../firmware/SolarBEMS/SolarBEMS.ino`](../firmware/SolarBEMS/SolarBEMS.ino) and
the thesis. Reference designators correspond to the system schematic.

> A more detailed **procurement workbook** with per-component engineering notes, supplier
> links, and cost tracking is available as [`Solar_BEMS_BOM.xlsx`](Solar_BEMS_BOM.xlsx). It is a
> **custom-PCB** design (discrete inductors, catch diodes, feedback dividers, headers) built
> around the same final specs — Arduino Mega 2560, 25 Wp panel, XR-121 3S 20 A BMS. The table
> below is the **as-built module-based perfboard** build (XL4015 / LM2596 modules). See
> [`schematic/README.md`](schematic/README.md) for the PCB-vs-perfboard distinction.

## Core modules & active devices

| Ref | Description | Value / Spec | Qty | Package |
| --- | --- | --- | --- | --- |
| ARD1 | Arduino Mega 2560 | ATmega2560, 16 MHz, 5 V | 1 | Dev board |
| U1 | INA219 Power Monitor | I²C, 26 V max, `0x40` | 1 | Breakout module |
| U2 | XL4015 CC/CV Buck | 8–36 V in, 0–5 A adj. | 1 | Module (3-screw) |
| U4 | LM2596 Buck Supply | 4.5–50 V in, 5 V out | 1 | Module |
| Q1 | IRLZ44N N-MOSFET | 55 V, 47 A, R<sub>DS</sub> = 22 mΩ | 1 | TO-220 |
| U3 | DS18B20 Temp Sensor | −55 to +125 °C, 1-Wire | 1 | Waterproof probe |
| OLED1 | SSD1306 OLED Display | 0.96″, 128×64 px, I²C `0x3C` | 1 | Module |

## Energy storage

| Ref | Description | Value / Spec | Qty | Package |
| --- | --- | --- | --- | --- |
| BT1–BT3 | 18650 Li-ion Cells | 3.7 V nom, ≥ 2000 mAh | 3 | Cylindrical |
| BH1 | 3S 18650 Holder | Series config, 3-cell | 1 | Plastic holder |
| BMS1 | XR-121 3S 20 A BMS | 4.2 V/cell OVP, 20 A | 1 | PCB module |

## Source & load

| Ref | Description | Value / Spec | Qty | Package |
| --- | --- | --- | --- | --- |
| PV1 | 25 Wp Solar Panel | V<sub>oc</sub> = 24.62 V, V<sub>mp</sub> = 20.84 V | 1 | 420 × 355 mm |
| LOAD1 | 12 V DC Fan | 12 V, ≤ 0.5 A, brushless | 1 | 80 mm PC fan |

## Protection

| Ref | Description | Value / Spec | Qty | Package |
| --- | --- | --- | --- | --- |
| F1 | 5 A Fuse + Holder | 250 V, fast-blow | 1 | 5×20 mm glass |
| D1 | 1N5822 Schottky | 40 V, 3 A, V<sub>f</sub> ≈ 0.32 V | 1 | DO-201AD |
| D4 | 1N4007 Rectifier | 1000 V, 1 A | 1 | DO-41 |
| TVS1 | P6KE24A TVS Diode | 24 V clamp, 600 W peak | 1 | DO-204AC |

## Capacitors

| Ref | Description | Value / Spec | Qty | Package |
| --- | --- | --- | --- | --- |
| C1 | Electrolytic | 470 µF / 35 V, low ESR 105 °C | 1 | Radial |
| C8 | Electrolytic | 470 µF / 25 V, battery bus bulk | 1 | Radial |
| C15 | Electrolytic | 100 µF / 25 V, load output cap | 1 | Radial |
| CMCU | Electrolytic | 100 µF / 16 V, MCU decoupling | 1 | Radial |
| C14 | Ceramic | 100 nF / 50 V, ADC RC filter | 1 | 0805 / Radial |

## Resistors & indicator

| Ref | Description | Value / Spec | Qty | Package |
| --- | --- | --- | --- | --- |
| R1, R2 | I²C SDA/SCL pull-ups | 4.7 kΩ | 2 | 1/4 W axial |
| R5 | DS18B20 DQ pull-up | 4.7 kΩ | 1 | 1/4 W axial |
| R8 | Voltage divider top | 10 kΩ | 1 | 1/4 W axial |
| R9 | Voltage divider bottom | 3.3 kΩ | 1 | 1/4 W axial |
| R10 | MOSFET gate series | 100 Ω | 1 | 1/4 W axial |
| R11 | MOSFET gate pull-down | 10 kΩ | 1 | 1/4 W axial |
| RLED | LED current limiter | 1 kΩ | 1 | 1/4 W axial |
| LED1 | Green LED, power-on | 3 mm | 1 | 3 mm THT |

## Connectors

| Ref | Description | Value / Spec | Qty | Package |
| --- | --- | --- | --- | --- |
| J1, J3 | Screw Terminals | 2-pin (PV in / load out) | 2 | 5.08 mm pitch |

---

### Key design notes

- **Voltage divider:** R8 = 10 kΩ, R9 = 3.3 kΩ → V<sub>BAT</sub> = V<sub>ADC</sub> × 4.03, scaling 12.6 V into the Arduino's 0–5 V ADC range.
- **I²C pull-ups:** R1/R2 (4.7 kΩ) on Board 2 are the **only** I²C pull-ups in the system — do **not** add extra pull-ups at the INA219 or OLED.
- **Charger calibration:** the XL4015 CV trim pot is set to 12.60 V and the CC trim pot to ~1.20 A, both verified against a dummy load **before** any battery connection.
- **Flyback diode:** D4 (1N4007) sits across the IRLZ44N drain–source to absorb the 12 V fan's inductive kick on switch-off.
