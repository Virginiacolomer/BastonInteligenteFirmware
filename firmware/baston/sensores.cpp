/* =============================================================================
 * sensores.cpp — Implementación · Dueño: H4 · CU01, CU02 · ver sensores.h
 * -----------------------------------------------------------------------------
 * Cada 30 ms se dispara UN sensor, alternando bajo / alto. Así cada HC-SR04
 * se dispara cada 60 ms (el fabricante pide ≥ 60 ms entre disparos) y el eco
 * de uno no lo lee el otro.
 * pulseIn() espera el eco hasta TIMEOUT_ECO_US (25 ms ≈ 4,3 m); si no llega,
 * se toma DIST_SIN_ECO_CM (400 cm = "no hay obstáculo").
 * ========================================================================== */
#include "config.h"
#include "sensores.h"

namespace sensores {

static uint16_t muestrasBajo[MUESTRAS_MEDIANA];
static uint16_t muestrasAlto[MUESTRAS_MEDIANA];
static uint8_t  idxBajo = 0, idxAlto = 0;
static uint16_t filtradoBajo = DIST_SIN_ECO_CM, filtradoAlto = DIST_SIN_ECO_CM;
static bool     turnoBajo = true;
static uint32_t ultimoMs = 0;

// Dispara un pulso de 10 µs en TRIG y devuelve la distancia en cm (o DIST_SIN_ECO_CM).
static uint16_t medir(uint8_t pinTrig, uint8_t pinEcho) {
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  unsigned long duracion = pulseIn(pinEcho, HIGH, TIMEOUT_ECO_US);
  if (duracion == 0) return DIST_SIN_ECO_CM;          // sin eco: nada adelante
  uint32_t cm = duracion / 58;                          // ida y vuelta a ~343 m/s
  if (cm < 2) cm = 2;                                   // mínimo del HC-SR04
  if (cm > DIST_SIN_ECO_CM) cm = DIST_SIN_ECO_CM;
  return (uint16_t)cm;
}

// Mediana de MUESTRAS_MEDIANA valores: descarta lecturas sueltas absurdas.
static uint16_t mediana(const uint16_t *v) {
  uint16_t c[MUESTRAS_MEDIANA];
  for (uint8_t i = 0; i < MUESTRAS_MEDIANA; i++) c[i] = v[i];
  for (uint8_t i = 1; i < MUESTRAS_MEDIANA; i++) {      // inserción: son 5 valores
    uint16_t x = c[i];
    int8_t j = i - 1;
    while (j >= 0 && c[j] > x) { c[j + 1] = c[j]; j--; }
    c[j + 1] = x;
  }
  return c[MUESTRAS_MEDIANA / 2];
}

void iniciar() {
  pinMode(PIN_TRIG_BAJO, OUTPUT);
  pinMode(PIN_TRIG_ALTO, OUTPUT);
  digitalWrite(PIN_TRIG_BAJO, LOW);
  digitalWrite(PIN_TRIG_ALTO, LOW);
  pinMode(PIN_ECHO_BAJO, INPUT);
  pinMode(PIN_ECHO_ALTO, INPUT);
  for (auto &m : muestrasBajo) m = DIST_SIN_ECO_CM;
  for (auto &m : muestrasAlto) m = DIST_SIN_ECO_CM;
}

void actualizar() {
  if (millis() - ultimoMs < PERIODO_MEDICION_MS) return;
  ultimoMs = millis();
  if (turnoBajo) {
    muestrasBajo[idxBajo] = medir(PIN_TRIG_BAJO, PIN_ECHO_BAJO);
    idxBajo = (idxBajo + 1) % MUESTRAS_MEDIANA;
    filtradoBajo = mediana(muestrasBajo);
  } else {
    muestrasAlto[idxAlto] = medir(PIN_TRIG_ALTO, PIN_ECHO_ALTO);
    idxAlto = (idxAlto + 1) % MUESTRAS_MEDIANA;
    filtradoAlto = mediana(muestrasAlto);
  }
  turnoBajo = !turnoBajo;
}

uint16_t distanciaBajo() { return filtradoBajo; }
uint16_t distanciaAlto() { return filtradoAlto; }

}  // namespace sensores
