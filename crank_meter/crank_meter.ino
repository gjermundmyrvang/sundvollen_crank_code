// Hand-crank generator energy meter
// Reads A0 (divider tap between R2 and R3), computes the generator
// voltage, instantaneous power dissipated in the R2+R3 divider (the
// only load on the generator), and integrates that into cumulative
// energy in Wh. Streams CSV over serial.
// Note: no rectifier is used. The generator is constrained to spin
// only in the direction that produces a positive voltage, so the
// output stays within the ADC's 0 to VREF range.

// ---- Circuit constants ----
const float R2 = 10000.0;   // top resistor of sensing divider (ohms)
const float R3 = 15000.0;   // bottom resistor of sensing divider (ohms)

const float VREF = 5.0;         // Arduino Uno ADC reference voltage
const int   ADC_MAX = 1023;     // 10-bit ADC

const unsigned long SAMPLE_INTERVAL_MS = 100; // how often to sample/report

// ---- State ----
unsigned long lastSampleTime = 0;
double cumulativeWh = 0.0;
double cumulativeJ = 0.0;

void setup() {
  Serial.begin(9600);
  lastSampleTime = millis();
  Serial.println("millis,VA0,Vgen,P_watts,cumulative_Wh,cumulative_J");
}

void loop() {
  unsigned long now = millis();
  if (now - lastSampleTime < SAMPLE_INTERVAL_MS) {
    return;
  }
  unsigned long dt_ms = now - lastSampleTime;
  lastSampleTime = now;

  int raw = analogRead(A0);
  float VA0 = (raw / (float)ADC_MAX) * VREF;

  // Reconstruct the generator voltage from the divider tap
  float Vgen = VA0 * (R2 + R3) / R3;

  // Power dissipated in the divider itself (R2 + R3 in series),
  // since it's the only load on the generator output
  float P = (Vgen * Vgen) / (R2 + R3);

  // Integrate power over the elapsed interval to accumulate energy
  // P (W) * dt (ms) / 1000 = Joules for this interval
  // Joules / 3600 = Wh for this interval
  double joulesThisStep = (double)P * (dt_ms / 1000.0);
  cumulativeWh += joulesThisStep / 3600.0;
  cumulativeJ += joulesThisStep;
  Serial.print(now);
  Serial.print(",");
  Serial.print(VA0, 4);
  Serial.print(",");
  Serial.print(Vgen, 4);
  Serial.print(",");
  Serial.print(P, 4);
  Serial.print(",");
  Serial.print(cumulativeWh, 8);
  Serial.print(",");
  Serial.println(cumulativeJ, 8);
}