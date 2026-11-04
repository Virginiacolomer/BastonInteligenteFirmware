/* =============================================================================
 * obstaculos.h — Decide cuándo y cómo avisar obstáculos
 * -----------------------------------------------------------------------------
 * Capa:          FUNCIÓN (caso de uso)
 * Dueño:         H4 · Detección de obstáculos   · Paquete fase 2: I1
 * Casos de uso:  CU01 Avisar obstáculo bajo  ·  CU02 Avisar obstáculo alto
 * Historias:     HU01 (vibración proporcional) · HU02 (pitido aéreo)
 * Pruebas:       P1 precisión ≤ 3 cm · P2 vibra desde ~150 cm y aumenta
 *                P3 no titila a 150 ± 3 cm (histéresis) · P4 pitido a 1,6 m de altura
 *
 * Qué hace (CU01):
 *   d = sensores::distanciaBajo();  límite = ajustes::distAvisoBajo()
 *   si d < límite → aviso ON;  si d > límite + HISTERESIS_CM → aviso OFF
 *   con aviso ON: u = KP·(límite − d), entre PWM_MOTOR_MIN y PWM_MOTOR_MAX
 *   → avisos::vibrarObstaculo(u)
 * Qué hace (CU02):
 *   si sensores::distanciaAlto() < ajustes::distAvisoAlto()
 *     → avisos::activar(PATRON_OBSTACULO_ALTO), si no → desactivar
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
