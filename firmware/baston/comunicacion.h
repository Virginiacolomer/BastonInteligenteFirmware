/* =============================================================================
 * comunicacion.h — Todo lo que viaja entre el bastón y el backend
 * -----------------------------------------------------------------------------
 * Capa:          FUNCIÓN · corre como TAREA de FreeRTOS en el NÚCLEO 0
 * Dueño:         H3 · Botón de pánico (+ comunicación)  · Paquete fase 2: I4
 * Revisa:        S1 (que el JSON respete el contrato del cap. 7)
 * Casos de uso:  CU08 Publicar telemetría
 *                CU05 Registrar alerta (envía los eventos de CU03, CU09, CU11)
 *                CU10 / CU12 (recibe comandos y ajustes en la respuesta)
 *
 * Ciclo (cada ajustes::periodoTelemetriaMs(), 60 s por defecto):
 *   1. GNSS: modem::gnssEncender(); esperar fix hasta TIMEOUT_GNSS_S (45 s);
 *      guardar posición en `estado`; modem::gnssApagar().
 *   2. Red: modem::redConectar().
 *   3. Enviar los eventos pendientes de `colaEventos` → POST /eventos
 *      (o /comandos/ack). Si el backend responde OK en un pánico:
 *      avisos::reproducir(PATRON_PANICO_CONFIRMADO); si falla:
 *      PATRON_ENVIO_FALLIDO y reintento con el MISMO idLocal (REINTENTO_EVENTO_MS).
 *   4. POST /telemetria con el JSON del cap. 7.
 *   5. Leer la respuesta: "comandos" → estado.sonarPendiente;
 *      "ajustes" → ajustes::aplicar().
 *   6. modem::redReposo() y esperar al próximo ciclo.
 *
 * ⚠ PRIORIDAD DE LOS EVENTOS: si llega un evento mientras se busca el GPS
 *   (paso 1), se corta la búsqueda y se pasa al paso 2 con la última posición
 *   conocida (estado.gpsMs → gps_edad_s). Revisar la cola durante la espera.
 * ⚠ Esta tarea PUEDE bloquearse (comandos AT). Nunca tocar sensores ni avisos
 *   directo desde acá: solo `estado`, `colaEventos`, `ajustes` y `avisos::reproducir`.
 * ========================================================================== */
#pragma once
#include <Arduino.h>

namespace comunicacion {
  void tarea(void *param);   // se crea en setup() con xTaskCreatePinnedToCore(..., núcleo 0)
}
