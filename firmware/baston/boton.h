/* =============================================================================
 * boton.h — Pulsador antipánico (lectura pura)
 * -----------------------------------------------------------------------------
 * Capa:          COMPONENTE
 * Dueño:         H3 · Botón de pánico
 * Casos de uso:  CU03 Activar alerta de pánico · CU10 (un toque corto silencia
 *                el «Encontrar mi bastón»)
 * Historias:     HU03 · Prueba P5
 *
 * Qué hace:
 *   - Lee IO11 con INPUT_PULLUP (presionado = LOW) y antirrebote de 30 ms.
 *   - Informa si está presionado, cuánto tiempo lleva presionado y si hubo un
 *     toque corto (< 1 s) desde la última consulta.
 *
 * Qué NO hace: la cuenta de 5 s ni los pitidos (eso es panico.cpp).
 * ========================================================================== */
#pragma once
#include <Arduino.h>

namespace boton {
  void     iniciar();
  void     actualizar();           // antirrebote; llamar en cada loop
  bool     presionado();           // estado estable actual
  uint32_t msPresionado();         // ms desde que se presionó (0 si está suelto)
  bool     huboToqueCorto();       // true una sola vez por cada toque < 1 s
}
