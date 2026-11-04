/* =============================================================================
 * sensores.h — Lectura de los dos ultrasónicos HC-SR04
 * -----------------------------------------------------------------------------
 * Capa:          COMPONENTE (solo mide; no decide si hay que avisar)
 * Dueño:         H4 · Detección de obstáculos
 * Casos de uso:  CU01 Avisar obstáculo bajo · CU02 Avisar obstáculo alto
 * Historias:     HU01, HU02 · Pruebas P1 (precisión ≤ 3 cm), P3, P4
 *
 * Qué hace:
 *   - Dispara los sensores ALTERNADOS cada PERIODO_MEDICION_MS (30 ms) para
 *     que el eco de uno no lo lea el otro.
 *   - Mide el ancho del pulso de ECHO con timeout de 25 ms (sin eco = 400 cm).
 *   - Aplica un filtro de MEDIANA de 5 muestras a cada sensor.
 *   - Expone la última distancia filtrada de cada uno, en cm.
 *
 * Qué NO hace: decidir avisos (obstaculos.cpp) ni escribir en `estado`.
 * Hardware:      ECHO entra por divisor 1 kΩ / 2 kΩ (nunca 5 V directo al pin).
 * Ojo:           pulseIn() bloquea hasta 25 ms; si molesta, pasar a medición
 *                por interrupciones (attachInterrupt + micros()).
 * ========================================================================== */
#pragma once
#include <Arduino.h>

namespace sensores {
  void     iniciar();            // configura TRIG como salida y ECHO como entrada
  void     actualizar();         // cada 30 ms mide UNO de los dos sensores (alternando)
  uint16_t distanciaBajo();      // cm filtrados, sensor inferior (DIST_SIN_ECO_CM si no hay eco)
  uint16_t distanciaAlto();      // cm filtrados, sensor superior
}
