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
 * Qué hace:
 *   d = la menor de sensores::distanciaBajo() y sensores::distanciaAlto()
 *   d < 50 cm ............... buzzer continuo  → PATRON_OBSTACULO_CERCA
 *   50 cm ≤ d < 1 m ......... pitido          → PATRON_OBSTACULO_MEDIO
 *   1 m ≤ d < límite ........ vibración proporcional → avisos::vibrarObstaculo(u)
 *                             (límite = ajustes::distAvisoBajo(), 150 cm por defecto)
 *   d ≥ límite .............. nada
 *   Con histéresis HISTERESIS_CM al pasar a un tramo más lejano.
 *   ⚠ Se aparta del patrón original del cap. 8 (CU01 vibración < 150 cm,
 *     CU02 pitido del sensor alto < 100 cm): avisar al grupo.
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
