/* =============================================================================
 * ajustes.cpp — Implementación · Dueño: H1 / I6 · CU12 · ver ajustes.h
 * Nota: los getters se leen desde los dos núcleos; los valores son chicos y se
 * escriben de a uno, así que alcanza con `volatile`. Si suman campos grandes,
 * proteger con un mutex como `estado`.
 * ========================================================================== */
#include "config.h"
#include "ajustes.h"
// #include <Preferences.h>   // TODO(H1): descomentar
// static Preferences prefs;

namespace ajustes {

static ValoresAjustes actual;
static volatile uint32_t contadorId = 0;

void iniciar() {
  // TODO(H1): prefs.begin("baston", false); leer cada campo con su valor por defecto
  //           y el contador de id_local.
}

bool aplicar(const ValoresAjustes &v) {
  // TODO(I6): validar cada campo según los rangos de ajustes.h; copiar los válidos
  //           en `actual`; guardar en NVS; devolver true si se aplicó algo.
  (void)v;
  return false;
}

uint16_t version()             { return actual.version; }
uint32_t tiempoPanicoMs()      { return actual.panicoS * 1000UL; }
uint16_t distAvisoBajo()       { return actual.distBajoCm; }
uint32_t periodoTelemetriaMs() { return actual.telemetriaS * 1000UL; }
bool     modoCasa()            { return actual.modoCasa; }

uint32_t siguienteIdLocal() {
  // TODO(H1): guardar el contador en NVS cada tanto (no en cada evento, para no gastar la flash).
  return ++contadorId;
}

}  // namespace ajustes
