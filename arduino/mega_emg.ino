/* Mega 2560 + 2 MyoWare, sorties ENV sur A0 et A1.
 * Acquisition et decision uniquement : AUCUNE sortie moteur.
 * Alimentation sur batterie. PC portable sur batterie sans liaison secteur.
 * Moniteur serie 115200 : r = repos, f = contraction flechisseurs,
 * e = contraction extenseurs. Tenir chaque etat 3 secondes apres la commande.
 * Valeurs de filtrage/seuils proposees pour essais, a valider experimentalement.
 */
#include <Arduino.h>
#include <math.h>

const byte N = 2;  // Mettre 1 pour commencer avec un seul capteur.
const byte EMG_PINS[2] = {A0, A1};
const unsigned long SAMPLE_US = 5000; // 200 Hz, sortie ENV seulement.
const unsigned long CONFIRM_MS = 80;
const float ALPHA = 0.20f;
float baseline[2] = {0, 0}, noise[2] = {0, 0};
float contraction[2] = {0, 0}, env[2] = {0, 0};
int rawValue[2] = {0, 0};
bool restOK = false, contractionOK[2] = {false, false};
bool active[2] = {false, false};
unsigned long lastSample = 0, lastPrint = 0, candidateSince = 0;
int candidate = 0, command = 0; // -1 ouvrir, 0 arret, +1 fermer

void resetDecision() {
  command = candidate = 0;
  candidateSince = millis();
  for (byte i = 0; i < N; ++i) active[i] = false;
  digitalWrite(LED_BUILTIN, LOW);
}

void measure(bool resting, byte channel) {
  resetDecision();
  Serial.println(F("Mesure 3 s : maintenir l'etat demande."));
  float sum[2] = {0, 0}, square[2] = {0, 0};
  bool saturated[2] = {false, false};
  for (int k = 0; k < 600; ++k) {
    for (byte i = 0; i < N; ++i) {
      int v = analogRead(EMG_PINS[i]);
      sum[i] += v;
      square[i] += (float)v * v;
      if (v >= 1020) saturated[i] = true;
    }
    delay(5); // Duree approximative : conversions ADC en plus des 5 ms.
  }
  if (resting) {
    restOK = true;
    for (byte i = 0; i < N; ++i) {
      baseline[i] = sum[i] / 600.0f;
      float variance = square[i] / 600.0f - baseline[i] * baseline[i];
      noise[i] = sqrtf(variance > 0 ? variance : 0);
      env[i] = baseline[i];
      contractionOK[i] = false;
      if (saturated[i]) restOK = false;
      Serial.print(F("Canal ")); Serial.print(i);
      Serial.print(F(" repos=")); Serial.print(baseline[i]);
      Serial.print(F(" ecart-type=")); Serial.println(noise[i]);
    }
    Serial.println(restOK ? F("Repos enregistre. Faire f puis e (si N=2).")
                          : F("Repos sature : verifier montage et refaire r."));
  } else {
    contraction[channel] = sum[channel] / 600.0f;
    float span = contraction[channel] - baseline[channel];
    // Criteres heuristiques, pas un diagnostic de qualite medicale.
    contractionOK[channel] = !saturated[channel] && span >= 20 && span > 10 * noise[channel];
    Serial.print(F("Canal ")); Serial.print(channel);
    Serial.print(F(" contraction=")); Serial.println(contraction[channel]);
    Serial.println(contractionOK[channel] ? F("Calibration acceptee.")
      : F("Ecart insuffisant/bruit/saturation : repositionner puis recalibrer."));
  }
  lastSample = micros();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  analogReference(DEFAULT);
  Serial.println(F("ENV uniquement. r=repos, f=flechisseurs, e=extenseurs."));
  Serial.println(F("Aucune commande de puissance : validation acquisition d'abord."));
  resetDecision();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'r') measure(true, 0);
    else if ((c == 'f' || c == 'e') && restOK) {
      byte i = c == 'f' ? 0 : 1;
      if (i < N) measure(false, i);
    }
  }
  unsigned long now = micros();
  if ((unsigned long)(now - lastSample) < SAMPLE_US) return;
  lastSample = now; // Cadence nominale, pas un enregistreur temps reel certifie.
  bool ready = restOK, clipping = false;
  for (byte i = 0; i < N; ++i) {
    rawValue[i] = analogRead(EMG_PINS[i]);
    env[i] += ALPHA * (rawValue[i] - env[i]);
    ready = ready && contractionOK[i];
    clipping = clipping || rawValue[i] >= 1020;
    if (contractionOK[i]) {
      float span = contraction[i] - baseline[i];
      float on = baseline[i] + 0.30f * span;
      float off = baseline[i] + 0.15f * span;
      if (!active[i] && env[i] >= on) active[i] = true;
      else if (active[i] && env[i] <= off) active[i] = false;
    }
  }
  int requested = 0;
  if (ready && !clipping) {
    if (N == 1) requested = active[0] ? 1 : 0; // Relachement = arret.
    else if (active[0] && !active[1]) requested = 1;
    else if (!active[0] && active[1]) requested = -1;
  }
  if (requested != candidate) {
    candidate = requested;
    candidateSince = millis();
    command = 0; // Pause pendant toute transition/reversion.
  }
  if (!ready || clipping || requested == 0) command = 0;
  else if ((unsigned long)(millis() - candidateSince) >= CONFIRM_MS) command = requested;
  digitalWrite(LED_BUILTIN, command != 0 ? HIGH : LOW);
  if ((unsigned long)(millis() - lastPrint) >= 50) {
    lastPrint = millis();
    Serial.print(F("env0:")); Serial.print(env[0]);
    if (N == 2) { Serial.print(F("\tenv1:")); Serial.print(env[1]); }
    Serial.print(F("\tcommande:")); Serial.print(command);
    Serial.print(F("\tcalibre:")); Serial.print(ready ? 1 : 0);
    Serial.print(F("\tsaturation:")); Serial.println(clipping ? 1 : 0);
  }
}
