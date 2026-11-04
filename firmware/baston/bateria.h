/* =============================================================================
 * bateria.h — Aviso de batería baja
 * -----------------------------------------------------------------------------
 * Capa:          FUNCIÓN (caso de uso)
 * Dueño:         H1 · Conexión y armado general  · Paquete fase 2: I7
 * Casos de uso:  CU11 Avisar batería baja  (incluye CU05 y CU04)
 * Historias:     HU06
 *
 * Qué hace (cada PERIODO_BATERIA_MS = 10 s):
 *   - Lee energia::bateriaPct(), bateriaMv() y cargando(); los copia en `estado`.
 *   - Al bajar de BATERIA_BAJA_PCT (20 %): reproducir(PATRON_BATERIA_BAJA) y
 *     publicarEvento(EVENTO_BATERIA_BAJA, pct). Repite el pitido cada 10 min.
 *   - Al bajar de BATERIA_CRITICA_PCT (10 %): segundo y último evento.
 *   - Si se carga por encima de BATERIA_REARME_PCT (30 %): rearma ambos avisos.
 *   → Un solo mensaje a los contactos por cada umbral y por cada descarga.
 * ========================================================================== */
#pragma once

namespace bateria {
  void iniciar();
  void actualizar();
}
