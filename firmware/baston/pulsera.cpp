/* =============================================================================
 * pulsera.cpp — Implementación · Dueño: H2 · CU09 · ver pulsera.h
 * ========================================================================== */
#include "config.h"
#include "pulsera.h"
#include "avisos.h"
#include "ajustes.h"
#include "estado.h"

namespace pulsera {

enum Fase : uint8_t { PUESTA, SUELTA, AVISADA };
static Fase     fase = PUESTA;
static uint32_t sueltaDesdeMs = 0;

void iniciar() { pinMode(PIN_PULSERA, INPUT_PULLUP); }

void actualizar() {
  // TODO(H2): leer digitalRead(PIN_PULSERA) (HIGH = suelta) con antirrebote
  //   y seguir la máquina de estados de pulsera.h. Respetar ajustes::modoCasa().
  (void)sueltaDesdeMs;
  { BloqueoEstado b; estado.pulseraPuesta = (fase == PUESTA); }
}

bool puesta() { return fase == PUESTA; }

}  // namespace pulsera
