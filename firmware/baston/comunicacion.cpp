/* =============================================================================
 * comunicacion.cpp — Implementación · Dueño: H3 · CU08, CU05 · ver comunicacion.h
 * Librería sugerida: ArduinoJson v7 para armar y leer los JSON.
 * ========================================================================== */
#include "config.h"
#include "comunicacion.h"
#include "modem.h"
#include "estado.h"
#include "ajustes.h"
#include "avisos.h"

namespace comunicacion {

// Arma el JSON de POST /api/v1/telemetria con una copia del estado (cap. 7).
static String jsonTelemetria(const Estado &e) {
  // TODO(H3): {"dispositivo","ts","lat","lon","gps_fix","gps_edad_s","bateria_mv",
  //            "bateria_pct","cargando","pulsera","dist_bajo_cm","dist_alto_cm",
  //            "senal_csq","ajustes_version"}  — lat/lon en null si no hay fix.
  (void)e;
  return "{}";
}

// Arma el JSON de POST /api/v1/eventos (cap. 7).
static String jsonEvento(const Evento &ev, const Estado &e) {
  // TODO(H3): {"dispositivo","id_local","tipo":"panico|pulsera|bateria_baja",
  //            "origen":"baston","ts","lat","lon","gps_edad_s"}
  (void)ev; (void)e;
  return "{}";
}

// Lee "comandos" y "ajustes" de la respuesta a la telemetría.
static void procesarRespuesta(const String &resp) {
  // TODO(H3/I6): si hay un comando "sonar": { BloqueoEstado b; estado.sonarPendiente = true;
  //              estado.idComandoSonar = id; }  ·  si "ajustes" != null → ajustes::aplicar(v)
  (void)resp;
}

static void buscarPosicion() {
  // TODO(H3): paso 1 del ciclo. Salir antes si llega un evento a colaEventos
  //           (uxQueueMessagesWaiting(colaEventos) > 0).
}

static void enviarEventos() {
  // TODO(H3): paso 3. xQueuePeek → POST → si OK, xQueueReceive para sacarlo de la cola.
}

void tarea(void *param) {
  (void)param;
#if USAR_MODEM
  modem::encender();
#endif
  for (;;) {
#if USAR_MODEM
    buscarPosicion();
    if (modem::redConectar(60000)) {
      enviarEventos();
      Estado copia; { BloqueoEstado b; copia = estado; }
      String resp;
      int codigo = modem::httpPost("/telemetria", jsonTelemetria(copia), resp);
      if (codigo == 200) procesarRespuesta(resp);
      modem::redReposo();
    }
#endif
    // Espera hasta el próximo ciclo, pero se despierta antes si llega un evento.
    // TODO(H3): reemplazar por una espera que mire colaEventos (p. ej. de a 500 ms).
    vTaskDelay(pdMS_TO_TICKS(ajustes::periodoTelemetriaMs()));
  }
}

}  // namespace comunicacion
