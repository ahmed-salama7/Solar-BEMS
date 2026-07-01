/*
 * ============================================================================
 *  Solar BEMS  —  Solar Panel-Based Battery Energy Management System
 *  Arduino Mega 2560 Firmware
 * ----------------------------------------------------------------------------
 *  Istanbul Arel University — Electrical & Electronics Engineering
 *  Bachelor's Graduation Project
 *
 *  Author      : Ahmed Salama
 *  Supervisor  : Dr. Oben Dağ
 *  Date        : June 2026
 *  License     : MIT (see LICENSE in repository root)
 * ----------------------------------------------------------------------------
 *  Function:
 *    - Perturb & Observe (P&O) MPPT on a 100 ms control loop
 *    - CC/CV lithium-ion charging supervision (3S, 12.60 V full)
 *    - Low-Voltage Disconnect (LVD) with 2.0 V hysteresis
 *    - Thermal fault protection (shutdown > 45 C)
 *    - State-of-Health estimation via Coulomb counting
 *    - Live OLED dashboard + serial telemetry
 *
 *  Required libraries (install via Arduino Library Manager):
 *    Wire (built-in), Adafruit_INA219, Adafruit_SSD1306,
 *    Adafruit_GFX, OneWire, DallasTemperature
 * ============================================================================
 */

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ── Pin Definitions ──────────────────────────────────
#define PIN_DS18B20   2     // DS18B20 1-Wire data
#define PIN_BUTTON    3     // Mode pushbutton (INPUT_PULLUP)
#define PIN_GATE      8     // MOSFET gate (fan load switch)
#define PIN_PWM_XL    9     // XL4015 EN -> MPPT duty cycle (Timer2 PWM)
#define PIN_VBAT_ADC  A0    // Battery voltage divider midpoint

// ── System Thresholds ────────────────────────────────
#define V_FULL        12.60f  // Battery full (CV -> float)
#define V_CC_CV       11.50f  // CC/CV transition
#define V_LVD_OFF      9.00f  // Low-voltage disconnect
#define V_LVD_ON      11.00f  // LVD reconnect hysteresis
#define V_PV_MIN       5.00f  // Min PV to run MPPT
#define T_FAULT       45.0f   // Thermal fault threshold (C)
#define T_RECOVER     40.0f   // Fault recovery temperature (C)

// ── MPPT Parameters ──────────────────────────────────
#define MPPT_STEP      1      // Duty cycle step per iteration (%)
#define MPPT_MIN       20     // Minimum duty cycle (%)
#define MPPT_MAX       90     // Maximum duty cycle (%)
#define LOOP_MS        100    // Main loop period (ms)

// ── ADC Scaling ──────────────────────────────────────
// Divider: R8=10k, R9=3.3k -> V_BAT = V_ADC * 4.0303
#define VDIV_RATIO    4.0303f
#define VREF          5.00f

// ── System State Enum ────────────────────────────────
enum SysState { FAULT, NIGHT, MPPT_CC, MPPT_CV,
                FLOAT_FULL, LOAD_OFF, LOAD_ON };

// ── Object Instantiation ─────────────────────────────
Adafruit_INA219 ina(0x40);
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
OneWire ow(PIN_DS18B20);
DallasTemperature ds(&ow);

// ── Global Variables ─────────────────────────────────
float vpv = 0, ipv = 0, ppv = 0, ppvPrev = 0, vbat = 0, temp = 0;
float soc = 100.0f, soh = 100.0f;
float coulombs_mAh = 0;
int   dutyCycle = 50;
bool  loadOn = false;
bool  faultActive = false;
bool  chargeMode = false;      // false = CC, true = CV
SysState state = NIGHT;
unsigned long lastLoop = 0;

// ─────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Wire.begin();
  ina.begin();
  ds.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay(); oled.display();
  pinMode(PIN_GATE, OUTPUT);   digitalWrite(PIN_GATE, LOW);
  pinMode(PIN_PWM_XL, OUTPUT);  analogWrite(PIN_PWM_XL, 0);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  Serial.println(F("Solar BEMS starting..."));
}

// ─────────────────────────────────────────────────────
void loop() {
  if (millis() - lastLoop < LOOP_MS) return;
  lastLoop = millis();

  readSensors();
  evaluateSafety();
  if (!faultActive) {
    runMPPT();
    manageCharging();
    manageLoad();
    updateSoH();
  }
  updateOLED();
  logSerial();
}

// ─────────────────────────────────────────────────────
void readSensors() {
  vpv = ina.getBusVoltage_V();
  ipv = ina.getCurrent_mA() / 1000.0f;
  ppv = vpv * ipv;
  int adc = analogRead(PIN_VBAT_ADC);
  vbat = (adc * VREF / 1023.0f) * VDIV_RATIO;
  ds.requestTemperatures();
  temp = ds.getTempCByIndex(0);
}

// ─────────────────────────────────────────────────────
void evaluateSafety() {
  if (temp > T_FAULT) {
    faultActive = true;
    analogWrite(PIN_PWM_XL, 0);
    digitalWrite(PIN_GATE, LOW);
    loadOn = false;
    state = FAULT;
    return;
  }
  if (faultActive && temp <= T_RECOVER)
    faultActive = false;
}

// ─────────────────────────────────────────────────────
void runMPPT() {
  if (vpv < V_PV_MIN) { state = NIGHT; return; }
  float dp = ppv - ppvPrev;
  if (dp >= 0) dutyCycle += MPPT_STEP;
  else         dutyCycle -= MPPT_STEP;
  dutyCycle = constrain(dutyCycle, MPPT_MIN, MPPT_MAX);
  analogWrite(PIN_PWM_XL, map(dutyCycle, 0, 100, 0, 255));
  ppvPrev = ppv;
}

// ─────────────────────────────────────────────────────
void manageCharging() {
  if (vpv < V_PV_MIN) return;        // night, skip
  if (vbat >= V_FULL) {
    analogWrite(PIN_PWM_XL, 0);
    state = FLOAT_FULL; return;
  }
  chargeMode = (vbat >= V_CC_CV);    // false = CC, true = CV
  state = chargeMode ? MPPT_CV : MPPT_CC;
  // Coulomb counting — charge phase
  coulombs_mAh += ipv * (LOOP_MS / 3600000.0f) * 1000.0f;
}

// ─────────────────────────────────────────────────────
void manageLoad() {
  if (vbat < V_LVD_OFF && loadOn) {
    digitalWrite(PIN_GATE, LOW);
    loadOn = false; state = LOAD_OFF;
  } else if (vbat > V_LVD_ON && !loadOn) {
    digitalWrite(PIN_GATE, HIGH);
    loadOn = true; state = LOAD_ON;
  }
}

// ─────────────────────────────────────────────────────
void updateSoH() {
  // Cells are in SERIES (3S): a series pack shares a single cell's mAh capacity
  // (series adds voltage, not charge capacity). Coulomb counting integrates the
  // series charge current, so SoH is referenced to the per-cell nominal, 2400 mAh.
  // (2140 mAh measured / 2400 mAh nominal ~= 89.2 %.)
  float C_NOMINAL_MAH = 2400.0f;
  soc = constrain((vbat - 9.0f) / (12.6f - 9.0f) * 100.0f, 0, 100);
  // SoH from measured vs nominal (simplified)
  if (coulombs_mAh > 0)
    soh = constrain((coulombs_mAh / C_NOMINAL_MAH) * 100.0f, 0, 100);
}

// ─────────────────────────────────────────────────────
const char* stateLabel() {
  switch (state) {
    case FAULT:       return "FAULT!";
    case NIGHT:       return "NIGHT";
    case MPPT_CC:     return "CHG-CC";
    case MPPT_CV:     return "CHG-CV";
    case FLOAT_FULL:  return "FULL";
    case LOAD_OFF:    return "LVD OFF";
    case LOAD_ON:     return "LOAD ON";
    default:          return "--";
  }
}

// ─────────────────────────────────────────────────────
void updateOLED() {
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  char buf[22];
  // Line 1: PV
  oled.setCursor(0, 0);
  snprintf(buf, 22, "PV %4.1fV %4.2fA %5.1fW", vpv, ipv, ppv);
  oled.print(buf);
  // Line 2: Battery
  oled.setCursor(0, 10);
  snprintf(buf, 22, "BAT %5.2fV SoC %3.0f%%", vbat, soc);
  oled.print(buf);
  // Line 3: Temperature + SoH
  oled.setCursor(0, 20);
  snprintf(buf, 22, "T%4.1fC SoH %5.1f%%", temp, soh);
  oled.print(buf);
  // Line 4: Status
  oled.setCursor(0, 30);
  snprintf(buf, 22, "Mode: %-10s", stateLabel());
  oled.print(buf);
  // Line 5: Duty cycle
  oled.setCursor(0, 40);
  snprintf(buf, 22, "MPPT duty: %3d%%", dutyCycle);
  oled.print(buf);
  oled.display();
}

// ─────────────────────────────────────────────────────
void logSerial() {
  Serial.print(F("Vpv:"));   Serial.print(vpv, 2);
  Serial.print(F(" Ipv:"));  Serial.print(ipv, 3);
  Serial.print(F(" Ppv:"));  Serial.print(ppv, 2);
  Serial.print(F(" Vbat:")); Serial.print(vbat, 2);
  Serial.print(F(" T:"));    Serial.print(temp, 1);
  Serial.print(F(" SoC:"));  Serial.print(soc, 1);
  Serial.print(F(" SoH:"));  Serial.print(soh, 1);
  Serial.print(F(" Mode:")); Serial.println(stateLabel());
}
// ── End of firmware ──────────────────────────────────
