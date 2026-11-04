/* =============================================================================
 * pulsera.h — Detección de pulsera suelta
 * -----------------------------------------------------------------------------
 * Capa:          FUNCIÓN (caso de uso; la lectura del pin es tan simple que va acá)
 * Dueño:         H2 · Pulsera desconectada       · Paquete fase 2: I5
 * Casos de uso:  CU09 Alertar pulsera suelta  (incluye CU05 y CU04)
 * Historias:     HU10 · Prueba P10
 * Hardware:      IO12 con INPUT_PULLUP → contacto del conector → GND
 *                enchufada = LOW · suelta = HIGH
 *
 * Máquina de estados:
 *   PUESTA ──(HIGH > ANTIRREBOTE_PULSERA_MS = 1 s)──► SUELTA:
 *        avisos::activar(PATRON_PULSERA_SUELTA)
 *   SUELTA ──(vuelve a LOW)──► PUESTA: desactivar el patrón
 *   SUELTA ──(pasan AVISO_PULSERA_MS = 30 s)──► AVISADA: publicarEvento(EVENTO_PULSERA)
 *   SUELTA/AVISADA ──(pasan SONIDO_PULSERA_MAX_MS = 60 s)──► deja de sonar
 *   Si ajustes::modoCasa() está activo: no se dispara nada (CU12).
 *
 * Además: copia el estado en estado.pulseraPuesta (viaja en la telemetría).
 * ========================================================================== */
#pragma once

namespace pulsera {
  void iniciar();
  void actualizar();
  bool puesta();
}
