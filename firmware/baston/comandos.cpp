/* =============================================================================
 * comandos.cpp — Implementación · Dueño: H3 / I6 · CU10 · ver comandos.h
 * ========================================================================== */
#include "config.h"
#include "comandos.h"
#include "avisos.h"
#include "boton.h"
#include "estado.h"

namespace comandos {

static bool     sonando = false;
static uint32_t desdeMs = 0;

void iniciar() {}

void actualizar() {
  bool pendiente; uint16_t id;
  { BloqueoEstado b; pendiente = estado.sonarPendiente; id = estado.idComandoSonar; estado.sonarPendiente = false; }
  // TODO(I6): si `pendiente`: activar PATRON_SONAR_BASTON, sonando = true, desdeMs = millis(),
  //           publicarEvento(EVENTO_ACK_COMANDO, id).
  //           Si sonando y (pasaron DURACION_SONAR_S o boton::huboToqueCorto()): desactivar.
  (void)pendiente; (void)id; (void)sonando; (void)desdeMs;
}

}  // namespace comandos
