# Arduino Mega 2560 — Pin Assignments

Complete pin map for the Solar BEMS, matching the firmware
[`../firmware/SolarBEMS/SolarBEMS.ino`](../firmware/SolarBEMS/SolarBEMS.ino).

| Mega Pin | Function | Mode | Notes |
| --- | --- | --- | --- |
| `5V` | Receive 5 V from LM2596 | — | Powers the Arduino and all peripherals |
| `GND` | Common ground | — | All three boards tie here |
| `2` | DS18B20 DQ (1-Wire) | INPUT | Temperature data from the probe |
| `3` | Mode pushbutton | INPUT_PULLUP | Active LOW, pin → GND; short press cycles display pages |
| `8` | IRLZ44N gate drive | OUTPUT | Switches the 12 V fan load ON/OFF |
| `9` (PWM) | XL4015 EN — MPPT | OUTPUT (PWM) | P&O duty-cycle output, Timer 2 |
| `20` (SDA) | I²C data bus | I²C | Shared: INA219 (`0x40`) + OLED (`0x3C`) |
| `21` (SCL) | I²C clock bus | I²C | Shared clock for all I²C devices |
| `A0` | Battery voltage ADC | ANALOG IN | From the R8/R9 divider midpoint |

## I²C bus map

| Device | Address | Pull-ups |
| --- | --- | --- |
| INA219 power monitor | `0x40` | R1/R2 on Board 2 (shared bus) |
| SSD1306 OLED display | `0x3C` | R1/R2 on Board 2 (shared bus) |

> The single INA219 sits on the **PV input** rail. A documented next step is adding a second
> INA219 at `0x41` (by bridging its A0 address pad) on the **battery bus** for accurate
> discharge-side Coulomb counting.

## ADC scaling

```
V_BAT = V_ADC × (R8 + R9) / R9 = V_ADC × (10k + 3.3k) / 3.3k ≈ V_ADC × 4.0303
```

At full charge (12.60 V) the divider midpoint sits at ≈ 3.13 V — safely inside the
Arduino's 0–5 V ADC range. C14 (100 nF) forms an RC filter on this node.
