/* =============================================================================
 * sensores.cpp — Implementación · Dueño: H4 · CU01, CU02 · ver sensores.h
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
  // TODO(H4): TRIG LOW 2 µs → HIGH 10 µs → LOW; pulseIn(pinEcho, HIGH, TIMEOUT_ECO_US);
  //           duración 0 → DIST_SIN_ECO_CM; si no, cm = duración / 58.
  (void)pinTrig; (void)pinEcho;
  return DIST_SIN_ECO_CM;
}

// Devuelve la mediana del buffer (copiar y ordenar 5 valores alcanza).
static uint16_t mediana(const uint16_t *v) {
  // TODO(H4): ordenar una copia de MUESTRAS_MEDIANA valores y devolver el del medio.
  return v[0];
}

void iniciar() {
  pinMode(PIN_TRIG_BAJO, OUTPUT);
  pinMode(PIN_TRIG_ALTO, OUTPUT);
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
#if DEBUG_SENSORES
  // Formato CSV para la prueba de cobertura: ms,bajo_cm,alto_cm
  Serial.printf("%lu,%u,%u\n", (unsigned long)millis(), filtradoBajo, filtradoAlto);
#endif
}

uint16_t distanciaBajo() { return filtradoBajo; }
uint16_t distanciaAlto() { return filtradoAlto; }

}  // namespace sensores
