/* =============================================================================
 * obstaculos.h — Decide cuándo y cómo avisar obstáculos
 * -----------------------------------------------------------------------------
 * Capa:          FUNCIÓN (caso de uso)
 * Dueño:         H4 · Detección de obstáculos   · Paquete fase 2: I1
 * Casos de uso:  CU01 Detectar obstáculos (pasos 4–5) · CU02 Avisar según la distancia
 * Historias:     HU01, HU02
 * Pruebas:       P2 el aviso empieza en el límite y se acelera al acercarse
 *                P3 no titila con el objeto quieto en un borde
 *                P4 una «rama» a 1,6 m de altura hace sonar el buzzer (sensor alto)
 *
 * Qué hace (cambia el cap. 8 de la rev. D, ver CAMBIO_AVISOS_OBSTACULOS.txt punto 8):
 *   Cada sensor tiene SU aviso, con la misma regla:
 *     sensor ALTO (cabeza/torso) → BUZZER
 *     sensor BAJO (suelo)        → MOTOR VIBRADOR
 *                       intermitente desde    fijo por debajo de
 *     ALTO (buzzer) ..... 120 cm                 80 cm   (cabeza: fijo antes)
 *     BAJO (motor) ...... 120 cm (ajuste CU12)   50 cm
 *   En el tramo intermitente: pulsos de 100 ms, 1 por segundo en el límite y
 *   5 por segundo justo antes del fijo (más cerca = más seguido).
 *   Histéresis de HISTERESIS_CM (8 cm) para apagar y para salir del continuo.
 *   Si suena algo de mayor prioridad (pánico, pulsera, sonar), el buzzer lo
 *   decide avisos.cpp; el motor solo lo pisan sonar y pánico enviado.
 *   ⚠ Si un sensor deja de responder (CU01 1a), su aviso queda apagado: el otro
 *     sensor NO lo cubre, porque cada uno avisa con un actuador distinto.
 * Además: copia ambas distancias en `estado` (viajan en la telemetría, CU08).
 *
 * ⚠ SEGURIDAD: este módulo nunca depende de la red. Tiene que funcionar aunque
 *   el módem esté colgado, sin batería en el chip o sin cobertura.
 * ========================================================================== */
#pragma once

namespace obstaculos {
  void iniciar();
  void actualizar();
}
