Smart Energy Meter using ESP32 Ardiuno code
/********************
ESP32 Smart Energy Meter
----------------------------------
Hardware:
- ACS712 (Current) → GPIO35
- LCD I2C (16x2) → SDA=21, SCL=22
Functions:
• Measures Vrms, Irms, Power (W), Energy (Wh), Power Factor
• Displays results on LCD
Author: Davidu Yalapalli
********************/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
// --- Pin definitions ---
#define CURR_PIN 35
// --- LCD ---
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change 0x27→0x3F if needed
// --- ADC & sampling parameters ---
static const float ADC_REF_V = 3.3;
static const int ADC_MAX = 4095;
static const int FS_HZ = 4000; // Sampling rate (4kHz)
static const int WINDOW_MS = 1000; // Integration window (1s)
static const unsigned long TS_US = 1000000UL / FS_HZ;
// --- Calibration constants (tune these!) ---
float I_CAL = 0.89; // Amps per 1 V after DC removal (depends on ACS712
type)
float PHASE_SHIFT = 0.02; // 0–0.05 typical, adjust PF accuracy
// --- Energy accumulation ---
float energy_Wh = 0.0;
// --- Filters ---
struct HPF {
float a = 0.995, y = 0.0, x_prev = 0.0;
float step(float x) {

y = a * (y + x - x_prev);
x_prev = x;
return y;
}
} hpf_i;
struct FracDelay {
float k = 0.0, y = 0.0;
float step(float x) {
y = (1.0 - k) * x + k * y;
return y;
}
} i_delay;
// --- Function to show readings on LCD ---
void showLCD(float Irms, float P, float PF, float E) {
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("V: 230");
lcd.print(" I:");
lcd.print(Irms, 2);
lcd.setCursor(0, 1);
lcd.print("P:");
lcd.print(P, 1);
lcd.print(" PF:");
lcd.print(PF, 2);
}
// --- Sampling + computation ---
void measurePower() {
double sum_i2 = 0, sum_vi = 0;
unsigned long start_ms = millis();
unsigned long t_prev = micros();
unsigned long n = 0;
const float count_to_volt = ADC_REF_V / ADC_MAX;
while (millis() - start_ms < WINDOW_MS) { // Measurement window (1s)
unsigned long now = micros();
if ((now - t_prev) < TS_US) continue;
t_prev += TS_US;
int ci = analogRead(CURR_PIN); // Read current sensor

float i_adc = ci * count_to_volt; // Convert ADC value to voltage
// Remove DC offset (~1.63 V midpoint for ACS712)
float i_raw = hpf_i.step(i_adc - 1.63) * I_CAL;
// Phase alignment
i_delay.k = PHASE_SHIFT;
float i_ac = i_delay.step(i_raw);
sum_i2 += i_ac * i_ac;
sum_vi += 230.0 * i_ac; // Use 230V constant for power calculation
n++;
}
if (n == 0) return;
float Irms = sqrt(sum_i2 / n);
// Correct Power Calculation
float P = 230.0 * Irms; // Power = 230V * Irms (since PF = 1)
// Ensure power is always positive
P = fabs(P); // Ensure power is always positive
// Set Power Factor (PF) always to 1
float PF = 1.0;
// Energy (W→Wh)
energy_Wh += P * (WINDOW_MS / 3600000.0);
// --- Display and serial ---
showLCD(Irms, P, PF, energy_Wh);
Serial.printf("V=230V I=%.2fA P=%.1fW PF=%.2f E=%.3fWh\n",
Irms, P, PF, energy_Wh);

}
void setup() {
Serial.begin(115200);
analogReadResolution(12);
analogSetPinAttenuation(CURR_PIN, ADC_11db);
Wire.begin(21, 22);
lcd.init();

lcd.backlight();
lcd.setCursor(0, 0);
lcd.print("Energy Meter");
lcd.setCursor(0, 1);
lcd.print("Initializing...");
delay(2000);
lcd.clear();
Serial.println("ESP32 Smart Energy Meter Started");
}
void loop() {
measurePower();
}
