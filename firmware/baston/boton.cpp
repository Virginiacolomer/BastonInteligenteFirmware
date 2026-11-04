/* =============================================================================
 * boton.cpp — Implementación · Dueño: H3 · CU03, CU10 · ver boton.h
 * ========================================================================== */
#include "config.h"
#include "boton.h"

namespace boton {

static bool     estable = false;     // true = presionado
static uint32_t desdeMs = 0;
static bool     toqueCorto = false;

void iniciar() { pinMode(PIN_BOTON, INPUT_PULLUP); }

void actualizar() {
  // TODO(H3): leer digitalRead(PIN_BOTON) == LOW; si cambia y se mantiene
  //           ANTIRREBOTE_BOTON_MS, actualizar `estable` y `desdeMs`.
  //           Al soltar: si estuvo presionado < 1000 ms → toqueCorto = true.
}

bool     presionado()   { return estable; }
uint32_t msPresionado() { return estable ? millis() - desdeMs : 0; }
bool     huboToqueCorto() { bool t = toqueCorto; toqueCorto = false; return t; }

}  // namespace boton
