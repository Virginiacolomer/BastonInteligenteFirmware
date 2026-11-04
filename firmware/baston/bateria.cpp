/* =============================================================================
 * bateria.cpp — Implementación · Dueño: H1 · CU11 · ver bateria.h
 * ========================================================================== */
#include "config.h"
#include "bateria.h"
#include "energia.h"
#include "avisos.h"
#include "estado.h"

namespace bateria {

static uint32_t ultimoMs = 0;
static bool avisoBajaHecho = false, avisoCriticaHecho = false;

void iniciar() {}

void actualizar() {
  if (millis() - ultimoMs < PERIODO_BATERIA_MS) return;
  ultimoMs = millis();
  uint8_t pct = energia::bateriaPct();
  { BloqueoEstado b; estado.bateriaPct = pct; estado.bateriaMv = energia::bateriaMv(); estado.cargando = energia::cargando(); }
  // TODO(H1): umbrales, eventos y rearme según bateria.h.
  (void)avisoBajaHecho; (void)avisoCriticaHecho;
}

}  // namespace bateria
