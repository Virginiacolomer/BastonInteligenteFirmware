/* =============================================================================
 * consola.cpp — Implementación · Dueño: H1 · ver consola.h
 * Guarda un bit por letra a–z. Lo demás (enter, números, etc.) se descarta.
 * ========================================================================== */
#include "consola.h"

namespace consola {

static uint32_t pendientes = 0;   // bit 0 = 'a' … bit 25 = 'z'

void actualizar() {
  while (Serial.available()) {
    int c = Serial.read();
    if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    if (c >= 'a' && c <= 'z') pendientes |= 1UL << (c - 'a');
  }
}

bool tecla(char c) {
  if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
  if (c < 'a' || c > 'z') return false;
  uint32_t bit = 1UL << (c - 'a');
  bool hubo = pendientes & bit;
  pendientes &= ~bit;
  return hubo;
}

}  // namespace consola
