/* =============================================================================
 * obstaculos.h — Decide cuándo y cómo avisar obstáculos
 * -----------------------------------------------------------------------------
 * Capa:          FUNCIÓN (caso de uso)
 * Dueño:         H4 · Detección de obstáculos   · Paquete fase 2: I1
 * Casos de uso:  CU01 Detectar obstáculos (pasos 4–5) · CU02 Avisar según la distancia
 * Historias:     HU01 (vibración 1–1,5 m) · HU02 (pitido < 1 m, continuo < 50 cm)
 * Pruebas:       P2 vibra desde ~150 cm y aumenta hasta 1 m
 *                P3 no titila con el objeto quieto en un borde de tramo
 *                P4 una «rama» a 1,6 m de altura se detecta a 1 m (sensor alto)
 *
 * Qué hace (cap. 8, rev. D):
 *   d = la menor de sensores::distanciaBajo() y sensores::distanciaAlto()
 *       (el aviso es el mismo si el obstáculo está abajo o a la altura de la cabeza)
 *   d < 50 cm ............... buzzer continuo  → PATRON_OBSTACULO_CERCA
 *   50 cm ≤ d < 1 m ......... pitido 120/400 ms → PATRON_OBSTACULO_MEDIO
 *   1 m ≤ d < límite ........ vibración proporcional PWM 255 (1 m) → 90 (límite)
 *                             (límite = ajustes::distAvisoBajo(), 150 cm por defecto)
 *   d ≥ límite .............. nada
 *   Al acercarse cambia de tramo enseguida; al alejarse, cuando d supera el
 *   borde + HISTERESIS_CM (8 cm).
 *   Si suena algo de mayor prioridad (pánico, pulsera, sonar), el buzzer lo
 *   decide avisos.cpp; la vibración de obstáculo sigue.
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
