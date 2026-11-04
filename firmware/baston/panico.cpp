/* =============================================================================
 * panico.cpp — Implementación · Dueño: H3 · CU03 · ver panico.h
 * ========================================================================== */
#include "config.h"
#include "panico.h"
#include "boton.h"
#include "avisos.h"
#include "ajustes.h"
#include "estado.h"

namespace panico {

enum Fase : uint8_t { REPOSO, CONTANDO, ENVIADO };
static Fase fase = REPOSO;

void iniciar() { fase = REPOSO; }

void actualizar() {
  // TODO(H3): implementar la máquina de estados de panico.h usando
  //   boton::presionado(), boton::msPresionado() y ajustes::tiempoPanicoMs().
  //   Al pasar a ENVIADO: avisos::desactivar(PATRON_CUENTA_PANICO);
  //                       avisos::reproducir(PATRON_PANICO_ENVIADO);
  //                       publicarEvento(EVENTO_PANICO);
  (void)fase;
}

}  // namespace panico
