# Interactive System Schematic

**[`index.html`](index.html)** is a standalone, pin-level interactive schematic of the
Solar BEMS. Open it in any modern browser (Chrome, Firefox, Edge) — no installation needed.

### Controls

- **Scroll** — zoom in / out
- **Drag** — pan around the canvas
- **Hover a component** — view its full engineering specification and justification
- Toggle **pin labels** and **net labels** from the top bar

It covers all 11 subsystem blocks with 40+ components and 80+ annotated connections.

---

## PCB design view vs. as-built

This schematic uses the **final system's specifications** — **Arduino Mega 2560**, pins
**9** (MPPT PWM), **8** (load switch), **20 / 21** (I²C SDA / SCL), a **25 Wp** panel
(V<sub>oc</sub> = 24.62 V, V<sub>mp</sub> = 20.84 V), and the **XR-121 3S 20 A** BMS — matching
the thesis, README, and firmware.

One intentional difference remains. The schematic (and the detailed
[`Solar_BEMS_BOM.xlsx`](../Solar_BEMS_BOM.xlsx) procurement workbook) draw the converters as
**discrete components** — individual inductors, catch diodes, feedback dividers, test points,
and programming headers — because they target a **custom KiCad PCB**, a documented future step
(see the thesis, *Limitations and Future Work*). The **as-built** system instead uses pre-built
**XL4015 and LM2596 modules** on a three-board perfboard, which encapsulate those discrete
parts internally.

| View | What it shows | Where |
| --- | --- | --- |
| **PCB design** (this schematic + `.xlsx`) | Discrete-component custom-PCB layout, final specs | this folder |
| **As-built** (module perfboard) | XL4015/LM2596 modules on three perfboards | [`../bill-of-materials.md`](../bill-of-materials.md) |

Same topology, same pins, same specs — only the packaging (discrete PCB vs. modules) differs.

For the authoritative as-built details, use:

- [`../bill-of-materials.md`](../bill-of-materials.md) — final component list
- [`../pin-assignments.md`](../pin-assignments.md) — final Mega 2560 pin map
- [`../../docs/Solar_BEMS_Thesis.pdf`](../../docs/Solar_BEMS_Thesis.pdf) — full thesis
