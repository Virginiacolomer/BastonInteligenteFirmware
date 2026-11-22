/* =============================================================================
 * sensores.cpp — Implementación · Dueño: H4 · CU01 · ver sensores.h
 * -----------------------------------------------------------------------------
 * Medición SIN BLOQUEAR (no usa pulseIn):
 *   - Cada PERIODO_MEDICION_MS (30 ms) se hace una "vuelta":
 *       1. se CIERRA la medición del sensor disparado en la vuelta anterior
 *          (hace 30 ms: el eco más largo útil, 400 cm, dura ~23 ms);
 *       2. se DISPARA el otro sensor (pulso de 10 µs en TRIG).
 *     Así se alternan bajo / alto, cada HC-SR04 se dispara cada 60 ms (lo que
 *     pide el fabricante) y el eco de uno no lo lee el otro.
 *   - El ancho del pulso de ECHO lo mide una interrupción (flanco de subida y
 *     de bajada con el reloj en µs). El loop nunca espera al sensor.
 *
 * Resultado de cada medición:
 *   - Eco completo de ≤ TIMEOUT_ECO_US ........ distancia = µs / 58
 *   - Eco más largo, o todavía en HIGH ......... DIST_SIN_ECO_CM (nada adelante)
 *   - El ECHO nunca subió ...................... DIST_SIN_ECO_CM y cuenta una
 *     falla. Con FALLAS_NO_RESPONDE seguidas el sensor se marca "no responde"
 *     (CU01 1a: cable suelto). El aviso sigue con el otro sensor.
 *   - Si al disparar el ECHO sigue en HIGH (algunos clones lo dejan alto
 *     ~200 ms cuando no hay eco), no se dispara: cuenta como sin obstáculo.
 * ========================================================================== */
#include "config.h"
#include "sensores.h"
#include <esp_timer.h>
#include <soc/gpio_reg.h>

#define FALLAS_NO_RESPONDE  5   // mediciones seguidas sin flanco de subida

namespace sensores {

struct Canal {
  uint8_t  trig, echo;
  // Escritos por la interrupción
  volatile int64_t subidaUs = 0, bajadaUs = 0;
  volatile bool    subio = false, bajo = false;
  // Escritos por el loop
  bool     armado = false;                 // se disparó y falta cerrar la medición
  uint16_t muestras[MUESTRAS_MEDIANA];
  uint8_t  idx = 0;
  uint16_t filtrado = DIST_SIN_ECO_CM;
  uint16_t ultimaCruda = DIST_SIN_ECO_CM;
  uint8_t  fallas = 0;
};

static Canal canalBajo, canalAlto;
static portMUX_TYPE candado = portMUX_INITIALIZER_UNLOCKED;   // loop ↔ interrupción
static bool     turnoBajo = true;
static uint32_t ultimoMs = 0;

// Lee el nivel del pin directo del registro: es seguro dentro de la interrupción
// aunque la flash esté ocupada (p. ej. mientras ajustes escribe en NVS).
// Los dos ECHO (IO17 e IO8) están por debajo de IO32.
static inline bool IRAM_ATTR nivel(uint8_t pin) {
  return (REG_READ(GPIO_IN_REG) >> pin) & 1;
}

static void IRAM_ATTR alCambiarEcho(void *arg) {
  Canal *c = static_cast<Canal *>(arg);
  int64_t ahora = esp_timer_get_time();
  portENTER_CRITICAL_ISR(&candado);
  if (nivel(c->echo)) {
    c->subidaUs = ahora;
    c->subio = true;
    c->bajo = false;
  } else if (c->subio && !c->bajo) {
    c->bajadaUs = ahora;
    c->bajo = true;
  }
  portEXIT_CRITICAL_ISR(&candado);
}

// Mediana de MUESTRAS_MEDIANA valores: descarta lecturas sueltas absurdas.
static uint16_t mediana(const uint16_t *v) {
  uint16_t c[MUESTRAS_MEDIANA];
  for (uint8_t i = 0; i < MUESTRAS_MEDIANA; i++) c[i] = v[i];
  for (uint8_t i = 1; i < MUESTRAS_MEDIANA; i++) {      // inserción: son pocos valores
    uint16_t x = c[i];
    int8_t j = i - 1;
    while (j >= 0 && c[j] > x) { c[j + 1] = c[j]; j--; }
    c[j + 1] = x;
  }
  return c[MUESTRAS_MEDIANA / 2];
}

static void guardarMuestra(Canal &c, uint16_t cm) {
  c.ultimaCruda = cm;
  c.muestras[c.idx] = cm;
  c.idx = (c.idx + 1) % MUESTRAS_MEDIANA;
  c.filtrado = mediana(c.muestras);
}

// Convierte el eco de la vuelta anterior en una muestra.
static void cerrar(Canal &c) {
  if (!c.armado) return;
  c.armado = false;

  portENTER_CRITICAL(&candado);
  bool    subio = c.subio, bajo = c.bajo;
  int64_t subida = c.subidaUs, bajada = c.bajadaUs;
  portEXIT_CRITICAL(&candado);

  if (!subio) {                                   // el sensor no contestó
    if (c.fallas < 255) c.fallas++;
    guardarMuestra(c, DIST_SIN_ECO_CM);
    return;
  }
  c.fallas = 0;
  if (!bajo) { guardarMuestra(c, DIST_SIN_ECO_CM); return; }   // eco todavía en HIGH

  int64_t us = bajada - subida;
  if (us <= 0 || us > TIMEOUT_ECO_US) { guardarMuestra(c, DIST_SIN_ECO_CM); return; }
  uint32_t cm = (uint32_t)us / 58;                // ida y vuelta a ~343 m/s
  if (cm < 2) cm = 2;                              // mínimo del HC-SR04
  if (cm > DIST_SIN_ECO_CM) cm = DIST_SIN_ECO_CM;
  guardarMuestra(c, (uint16_t)cm);
}

static void disparar(Canal &c) {
  if (nivel(c.echo)) {                            // sigue ocupado con el eco anterior
    guardarMuestra(c, DIST_SIN_ECO_CM);
    return;
  }
  portENTER_CRITICAL(&candado);
  c.subio = false;
  c.bajo = false;
  portEXIT_CRITICAL(&candado);

  digitalWrite(c.trig, HIGH);
  delayMicroseconds(10);                          // pulso de disparo (10 µs, no bloquea en la práctica)
  digitalWrite(c.trig, LOW);
  c.armado = true;
}

static void iniciarCanal(Canal &c, uint8_t trig, uint8_t echo) {
  c.trig = trig;
  c.echo = echo;
  for (auto &m : c.muestras) m = DIST_SIN_ECO_CM;
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  // PULLDOWN: si se suelta el cable del pin, lee LOW (y no ruido) → "no responde".
  pinMode(echo, INPUT_PULLDOWN);
  attachInterruptArg(digitalPinToInterrupt(echo), alCambiarEcho, &c, CHANGE);
}

void iniciar() {
  iniciarCanal(canalBajo, PIN_TRIG_BAJO, PIN_ECHO_BAJO);
  iniciarCanal(canalAlto, PIN_TRIG_ALTO, PIN_ECHO_ALTO);
  ultimoMs = millis();
}

void actualizar() {
  if (millis() - ultimoMs < PERIODO_MEDICION_MS) return;
  ultimoMs = millis();
  Canal &anterior = turnoBajo ? canalAlto : canalBajo;
  Canal &actual   = turnoBajo ? canalBajo : canalAlto;
  cerrar(anterior);
  disparar(actual);
  turnoBajo = !turnoBajo;
}

uint16_t distanciaBajo()     { return canalBajo.filtrado; }
uint16_t distanciaAlto()     { return canalAlto.filtrado; }
uint16_t crudaBajo()         { return canalBajo.ultimaCruda; }
uint16_t crudaAlto()         { return canalAlto.ultimaCruda; }
bool     respondeBajo()      { return canalBajo.fallas < FALLAS_NO_RESPONDE; }
bool     respondeAlto()      { return canalAlto.fallas < FALLAS_NO_RESPONDE; }

}  // namespace sensores
