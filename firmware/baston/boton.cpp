/* =============================================================================
 * boton.cpp — Implementación · Dueño: H3 · CU03, CU10 · ver boton.h
 * -----------------------------------------------------------------------------
 * Antirrebote: el pin tiene que quedarse ANTIRREBOTE_BOTON_MS (30 ms) en el
 * nuevo nivel para que cuente. `desdeMs` guarda cuándo empezó realmente la
 * pulsación (no cuándo se confirmó), así la cuenta de pánico no pierde 30 ms.
 * ========================================================================== */
#include "config.h"
#include "boton.h"

#define TOQUE_CORTO_MS  1000   // soltar antes de 1 s = toque corto (CU10 4a)

namespace boton {

static bool     estable = false;     // true = presionado (ya filtrado)
static bool     crudoAnterior = false;
static uint32_t cambioMs = 0;        // cuándo cambió por última vez la lectura cruda
static uint32_t desdeMs = 0;         // cuándo empezó la pulsación actual
static bool     toqueCorto = false;

void iniciar() {
  pinMode(PIN_BOTON, INPUT_PULLUP);
  crudoAnterior = estable = (digitalRead(PIN_BOTON) == LOW);
  cambioMs = desdeMs = millis();
}

void actualizar() {
  bool crudo = digitalRead(PIN_BOTON) == LOW;
  uint32_t ahora = millis();
  if (crudo != crudoAnterior) {        // rebote o cambio real: empezar a contar
    crudoAnterior = crudo;
    cambioMs = ahora;
    return;
  }
  if (crudo == estable || ahora - cambioMs < ANTIRREBOTE_BOTON_MS) return;

  estable = crudo;                     // cambio confirmado
  if (estable) {
    desdeMs = cambioMs;
  } else if (cambioMs - desdeMs < TOQUE_CORTO_MS) {
    toqueCorto = true;
  }
}

bool     presionado()   { return estable; }
uint32_t msPresionado() { return estable ? millis() - desdeMs : 0; }
bool     huboToqueCorto() { bool t = toqueCorto; toqueCorto = false; return t; }

}  // namespace boton
