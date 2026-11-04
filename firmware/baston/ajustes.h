/* =============================================================================
 * ajustes.h — Parámetros configurables desde la app, guardados en flash (NVS)
 * -----------------------------------------------------------------------------
 * Capa:          FUNCIÓN (caso de uso)
 * Dueño:         H1 (integración) · Paquete fase 2: I6
 * Casos de uso:  CU12 Configurar ajustes del bastón
 * Historias:     HU09 · Prueba P9 (los cambios llegan en la próxima telemetría)
 * Librería:      Preferences (viene con el core esp32)
 *
 * Qué hace:
 *   - Al arrancar, lee los ajustes guardados; si no hay, usa los de config.h.
 *   - comunicacion.cpp llama a aplicar() cuando la respuesta de telemetría trae
 *     "ajustes" (cap. 7). Valida rangos, guarda en flash y sube la versión.
 *   - Los demás módulos leen los valores con los getters (no usan las
 *     constantes de config.h directamente para estos parámetros).
 *   - Lleva el contador de id_local de los eventos (persistente: sobrevive a
 *     reinicios, así el backend no confunde eventos nuevos con reintentos).
 *
 * Rangos válidos (fuera de rango → se ignora ese valor):
 *   panico_s 3–10 · dist_bajo_cm 50–200 · dist_alto_cm 50–150
 *   telemetria_s 30–300 · modo_casa true/false
 * ========================================================================== */
#pragma once
#include <Arduino.h>
#include "config.h"

struct ValoresAjustes {
  uint16_t version       = 0;
  uint8_t  panicoS       = TIEMPO_PANICO_MS / 1000;
  uint16_t distBajoCm    = DIST_AVISO_BAJO_CM;
  uint16_t distAltoCm    = DIST_AVISO_ALTO_CM;
  uint16_t telemetriaS   = TELEMETRIA_S;
  bool     modoCasa      = false;
};

namespace ajustes {
  void     iniciar();                         // carga desde NVS
  bool     aplicar(const ValoresAjustes &v);  // valida, guarda y aplica (desde comunicacion)

  uint16_t version();
  uint32_t tiempoPanicoMs();
  uint16_t distAvisoBajo();
  uint16_t distAvisoAlto();
  uint32_t periodoTelemetriaMs();
  bool     modoCasa();

  uint32_t siguienteIdLocal();                // para Evento.idLocal
}
