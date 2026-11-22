/* =============================================================================
 * comandos.h — Ejecuta los comandos que llegan del backend
 * -----------------------------------------------------------------------------
 * Capa:          FUNCIÓN (caso de uso) · núcleo 1
 * Dueño:         H3 · Paquete fase 2: I6
 * Casos de uso:  CU10 Hacer sonar el bastón desde la app («Encontrar mi bastón»)
 * Historias:     HU07 · Prueba P8 (suena en ≤ un intervalo de telemetría + 10 s)
 *
 * Recorrido completo de CU10:
 *   App ── POST /comandos {tipo:"sonar"} ──► backend (queda pendiente)
 *   Bastón ── POST /telemetria ──► respuesta {"comandos":[{"id":41,"tipo":"sonar","duracion_s":30}]}
 *     comunicacion.cpp (núcleo 0) lee la respuesta y deja en `estado`:
 *       sonarPendiente = true · idComandoSonar = 41 · duracionSonarS = 30
 *     comandos.cpp (núcleo 1, este módulo):
 *       1. Activa PATRON_SONAR_BASTON: pitido + vibración 500 ms cada 1 s.
 *       2. publicarEvento(EVENTO_ACK_COMANDO, id) → comunicacion lo manda a
 *          POST /comandos/ack y la app anuncia «El bastón está sonando».
 *       3. Lo apaga al terminar la duración (30 s por defecto) o
 *          4a. con un toque corto al pulsador (boton::huboToqueCorto()).
 *
 * Detalles:
 *   - Si llega OTRA VEZ el mismo id (el backend no recibió el ack todavía), no
 *     vuelve a sonar: el ack sigue en la cola y se reintenta solo.
 *   - Si llega un id nuevo mientras suena, vuelve a empezar la cuenta.
 *   - Prioridad del buzzer (avisos.h): pánico, cuenta de pánico y pulsera
 *     suenan por encima; el sonar está por encima de los obstáculos.
 *   - DEBUG_COMANDOS: la tecla 'b' del monitor serie simula el comando (sin ack),
 *     para probar sin app ni red. Otra 'b' lo apaga.
 *   Comandos futuros (si se agregan al contrato) se suman acá.
 * ========================================================================== */
#pragma once
#include <Arduino.h>

namespace comandos {
  void iniciar();
  void actualizar();
  bool sonando();            // true mientras suena «Encontrar mi bastón»
}
