// ===================== PINS =====================
const int adcA = 34;     // Channel A TIA -> ADC
const int spA  = 27;     // Channel A comparator -> digital

const int adcB = 35;     // Channel B TIA -> ADC  (στον κώδικά σου)
const int spB  = 26;     // Channel B comparator -> digital (στον κώδικά σου)

// ===================== LOGGER TUNING =====================
const unsigned long LOG_PERIOD_MS = 1000;    // όπως στον κώδικά σου (1 γραμμή/δευτ.)
const unsigned long DIR_WINDOW_MS = 150;     // χρονικό παράθυρο για L/R (δοκίμασε 100–250ms)

// ===================== REFRACTORY =====================
const unsigned long REFRACTORY_US = 80000;   // όπως έχεις (80ms)

// ===================== SPIKE STATE =====================
volatile unsigned long spikesA = 0;
volatile unsigned long spikesB = 0;

volatile unsigned long lastUsA = 0;
volatile unsigned long lastUsB = 0;

// timestamps (millis) για direction
volatile unsigned long lastMsA = 0;
volatile unsigned long lastMsB = 0;

// για να μην τυπώνει το ίδιο direction ξανά και ξανά
unsigned long lastPairMsReported = 0;

// ===================== ISR =====================
void IRAM_ATTR isrA() {
  unsigned long now = micros();
  if (now - lastUsA > REFRACTORY_US) {
    spikesA++;
    lastUsA = now;
    lastMsA = millis();
  }
}

void IRAM_ATTR isrB() {
  unsigned long now = micros();
  if (now - lastUsB > REFRACTORY_US) {
    spikesB++;
    lastUsB = now;
    lastMsB = millis();
  }
}

// ===================== ADC AVERAGE (όπως το δικό σου) =====================
int readAvg(int pin, int n = 6) {
  long acc = 0;
  for (int i = 0; i < n; i++) {
    acc += analogRead(pin);
    delayMicroseconds(120);
  }
  return acc / n;
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);

  pinMode(spA, INPUT_PULLDOWN);
  pinMode(spB, INPUT_PULLDOWN);

  attachInterrupt(digitalPinToInterrupt(spA), isrA, FALLING);  // μετράμε πτώσεις
  attachInterrupt(digitalPinToInterrupt(spB), isrB, FALLING);

  analogReadResolution(12);
  analogSetPinAttenuation(adcA, ADC_11db);
  analogSetPinAttenuation(adcB, ADC_11db);

  // CSV header
  Serial.println("t_ms,VA,VB,spA_s,spB_s,dir");
}

// ===================== LOOP =====================
void loop() {
  static unsigned long t0 = millis();

  // ---- Analog reads ----
  int rawA = readAvg(adcA);
  int rawB = readAvg(adcB);

  float vA = (rawA / 4095.0f) * 3.3f;
  float vB = (rawB / 4095.0f) * 3.3f;

  // ---- Log every LOG_PERIOD_MS ----
  if (millis() - t0 >= LOG_PERIOD_MS) {

    // snapshot spike counts
    noInterrupts();
    unsigned long sA = spikesA; spikesA = 0;
    unsigned long sB = spikesB; spikesB = 0;

    unsigned long aMs = lastMsA;
    unsigned long bMs = lastMsB;
    interrupts();

    // spikes per second for this window
    float spsA = sA * (1000.0f / LOG_PERIOD_MS);
    float spsB = sB * (1000.0f / LOG_PERIOD_MS);

    // ---- Direction logic ----
    // Αν τα δύο events έγιναν κοντά χρονικά (μέσα στο window), βγάλε L/R
    // και μόνο μία φορά ανά “ζευγάρι” γεγονότων.
    char dir = '.';

    unsigned long nowMs = millis();
    unsigned long newest = (aMs > bMs) ? aMs : bMs;
    unsigned long delta  = (aMs > bMs) ? (aMs - bMs) : (bMs - aMs);

    bool pairClose = (aMs != 0 && bMs != 0 && delta <= DIR_WINDOW_MS);

    // επιπλέον: να είναι πρόσφατο (για να μη "κολλάει" παλιό pair)
    bool pairRecent = (newest != 0 && (nowMs - newest) <= (LOG_PERIOD_MS + DIR_WINDOW_MS));

    if (pairClose && pairRecent && newest != lastPairMsReported) {
      // ποιο ήρθε πρώτο;
      if (aMs < bMs) dir = 'R';   // A πρώτα, μετά B  => κίνηση A->B (π.χ. Left->Right)
      else           dir = 'L';   // B πρώτα, μετά A  => Right->Left
      lastPairMsReported = newest;
    }

    // ---- Print CSV line ----
    Serial.print(millis()); Serial.print(',');
    Serial.print(vA, 3);    Serial.print(',');
    Serial.print(vB, 3);    Serial.print(',');
    Serial.print(spsA, 2);  Serial.print(',');
    Serial.print(spsB, 2);  Serial.print(',');
    Serial.println(dir);

    t0 += LOG_PERIOD_MS;
  }

  delay(5);
}
