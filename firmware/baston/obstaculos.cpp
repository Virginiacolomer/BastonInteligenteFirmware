/* =============================================================================
 * obstaculos.cpp — Implementación · Dueño: H4 · CU01, CU02 · ver obstaculos.h
 * ========================================================================== */
#include "config.h"
#include "obstaculos.h"
#include "sensores.h"
#include "avisos.h"
#include "ajustes.h"
#include "estado.h"

namespace obstaculos {

static bool avisoBajoActivo = false;

void iniciar() {}

void actualizar() {
  uint16_t dBajo = sensores::distanciaBajo();
  uint16_t dAlto = sensores::distanciaAlto();

  // --- CU01 · obstáculo bajo, con histéresis ---
  // TODO(H4): implementar la regla de obstaculos.h y llamar a avisos::vibrarObstaculo(u).
  (void)avisoBajoActivo;

  // --- CU02 · obstáculo alto ---
  // TODO(H4): activar / desactivar PATRON_OBSTACULO_ALTO según ajustes::distAvisoAlto().

  { BloqueoEstado b; estado.distBajo = dBajo; estado.distAlto = dAlto; }
}

}  // namespace obstaculos
