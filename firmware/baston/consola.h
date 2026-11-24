/* =============================================================================
 * consola.h — Teclas de prueba por el monitor serie (solo depuración)
 * -----------------------------------------------------------------------------
 * Capa:          COMPONENTE
 * Dueño:         H1 (integración)
 *
 * Por qué existe: si cada módulo leyera Serial por su cuenta, el primero se
 * comería las teclas de los demás. consola lee todo una vez por loop y cada
 * módulo pregunta solo por SU tecla.
 *
 * Teclas en uso (minúsculas o mayúsculas, da igual):
 *   s ... activa / pausa el CSV de sensores (arranca en pausa) (obstaculos, DEBUG_SENSORES)
 *   r ... una lectura: distancias, crudas, zona y PWM (obstaculos, DEBUG_SENSORES)
 *   z ... prueba el buzzer solo, 1 s               (avisos,     DEBUG_AVISOS)
 *   m ... prueba el motor solo, 1 s                (avisos,     DEBUG_AVISOS)
 *   b ... simula «Encontrar mi bastón» (CU10)      (comandos,   DEBUG_COMANDOS)
 * Al agregar una tecla, sumarla a esta lista para no repetirla.
 * ========================================================================== */
#pragma once
#include <Arduino.h>

namespace consola {
  void actualizar();         // lee lo que llegó por Serial (llamar al principio del loop)
  bool tecla(char c);        // true UNA vez si se apretó la letra c (a–z) desde la última consulta
}
