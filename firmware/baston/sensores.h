/* =============================================================================
 * sensores.h — Lectura de los dos ultrasónicos HC-SR04
 * -----------------------------------------------------------------------------
 * Capa:          COMPONENTE (solo mide; no decide si hay que avisar)
 * Dueño:         H4 · Detección de obstáculos   · Paquete fase 2: I1
 * Casos de uso:  CU01 Detectar obstáculos
 * Historias:     HU01, HU02 · Pruebas P1 (error ≤ 3 cm), P4 (rama a 1,6 m)
 *
 * Qué hace (CU01, flujo principal):
 *   1. Dispara un sensor cada PERIODO_MEDICION_MS (30 ms), ALTERNANDO bajo y
 *      alto para que no se interfieran.
 *   2. Si no hay eco en 25 ms, toma 400 cm (sin obstáculo).
 *   3. Aplica un filtro de MEDIANA de 5 muestras a cada sensor.
 *   (Los pasos 4 y 5 —el aviso de cada sensor— están en obstaculos.)
 *   1a. Sensor que no responde (cable suelto): marca 400 cm y lo informa con
 *       respondeBajo()/respondeAlto(); su aviso queda apagado (alto = buzzer,
 *       bajo = motor: el otro sensor no lo reemplaza).
 *
 * No bloquea: el ancho del ECHO se mide con interrupciones (sin pulseIn).
 * Qué NO hace: decidir avisos (obstaculos.cpp) ni escribir en `estado`.
 * Hardware:      ECHO por divisor 1 kΩ (arriba) / 2,2 kΩ (abajo) → 3,44 V.
 *                Nunca 5 V directo al pin.
 *                Sensor bajo: TRIG IO16 · ECHO IO17 (tubo, ~62 cm de la punta)
 *                Sensor alto: TRIG IO18 · ECHO IO8  (bajo el mango, ~30°)
 * ========================================================================== */
#pragma once
#include <Arduino.h>

namespace sensores {
  void     iniciar();            // TRIG como salida, ECHO como entrada con interrupción
  void     actualizar();         // cada 30 ms cierra una medición y dispara el otro sensor

  uint16_t distanciaBajo();      // cm filtrados (mediana), sensor inferior · 400 = nada
  uint16_t distanciaAlto();      // cm filtrados (mediana), sensor superior
  uint16_t crudaBajo();          // última lectura sin filtrar (para calibrar, P1)
  uint16_t crudaAlto();
  bool     respondeBajo();       // false = 5 disparos seguidos sin respuesta (CU01 1a)
  bool     respondeAlto();
}
