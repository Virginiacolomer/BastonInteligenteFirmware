/* =============================================================================
 * comunicacion.cpp — Implementación · Dueño: H3 · CU08, CU05 · ver comunicacion.h
 * Librería: ArduinoJson v7 (Gestor de librerías) para armar y leer los JSON.
 * Hecho para CU10: lectura de "comandos" en la respuesta y envío del ack.
 * ========================================================================== */
#include "config.h"
#include "comunicacion.h"
#include "modem.h"
#include "estado.h"
#include "ajustes.h"
#include "avisos.h"
#include <ArduinoJson.h>

namespace comunicacion {

// Arma el JSON de POST /api/v1/telemetria con una copia del estado (cap. 7).
static String jsonTelemetria(const Estado &e) {
  // TODO(H3): {"dispositivo","ts","lat","lon","gps_fix","gps_edad_s","bateria_mv",
  //            "bateria_pct","cargando","pulsera","dist_bajo_cm","dist_alto_cm",
  //            "senal_csq","ajustes_version"}  — lat/lon en null si no hay fix.
  //            dist_bajo_cm/dist_alto_cm acá son las MEDICIONES (estado.distBajo/distAlto).
  //            En la respuesta, "ajustes" ya no trae dist_alto_cm (se eliminó).
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

// JSON de POST /api/v1/comandos/ack (CU10 paso 4).
// ⚠ El cuerpo del ack no está definido en el cap. 7: este formato lo tiene que
//   confirmar S1 (dueño del contrato).
static String jsonAck(const Evento &ev) {
  JsonDocument doc;
  doc["dispositivo"] = ID_DISPOSITIVO;
  doc["id"]          = ev.dato;          // id del comando que se ejecutó
  doc["estado"]      = "ejecutado";
  String s;
  serializeJson(doc, s);
  return s;
}

// Lee "comandos" y "ajustes" de la respuesta a la telemetría.
// Devuelve true si llegó algún comando (para mandar el ack sin esperar al próximo ciclo).
static bool procesarRespuesta(const String &resp) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, resp);
  if (err) {
    Serial.printf("[comunicacion] respuesta no es JSON válido: %s\n", err.c_str());
    return false;
  }

  // --- "comandos": [ { "id": 41, "tipo": "sonar", "duracion_s": 30 } ] (CU10) ---
  bool huboComando = false;
  for (JsonObject c : doc["comandos"].as<JsonArray>()) {
    const char *tipo = c["tipo"] | "";
    uint32_t id = c["id"] | 0UL;
    if (strcmp(tipo, "sonar") == 0) {
      int dur = c["duracion_s"] | DURACION_SONAR_S;
      if (dur < DURACION_SONAR_MIN_S || dur > DURACION_SONAR_MAX_S) dur = DURACION_SONAR_S;
      { BloqueoEstado b;
        estado.sonarPendiente = true;
        estado.idComandoSonar = id;
        estado.duracionSonarS = (uint16_t)dur; }
      huboComando = true;
#if DEBUG_COMUNICACION
      Serial.printf("[comunicacion] comando sonar id %lu, %d s\n", (unsigned long)id, dur);
#endif
    } else {
      Serial.printf("[comunicacion] comando desconocido \"%s\" (id %lu): se ignora\n", tipo, (unsigned long)id);
    }
  }

  // --- "ajustes": {...} o null (CU12) ---
  // TODO(I6/CU12): si doc["ajustes"] no es null, copiar a ValoresAjustes y ajustes::aplicar(v).
  return huboComando;
}

// Envía un evento de alerta (pánico, pulsera, batería) a POST /eventos. true si el backend lo registró.
static bool enviarAlerta(const Evento &ev) {
  // TODO(H3): Estado copia; { BloqueoEstado b; copia = estado; }
  //           codigo = modem::httpPost("/eventos", jsonEvento(ev, copia), resp);
  //           ok = codigo == 200 || codigo == 201;
  //           si ev.tipo == EVENTO_PANICO: avisos::reproducir(ok ? PATRON_PANICO_CONFIRMADO
  //                                                              : PATRON_ENVIO_FALLIDO);
  (void)ev;
  return false;
}

static bool enviarUno(const Evento &ev) {
  if (ev.tipo == EVENTO_ACK_COMANDO) {
    String resp;
    int codigo = modem::httpPost("/comandos/ack", jsonAck(ev), resp);
#if DEBUG_COMUNICACION
    Serial.printf("[comunicacion] ack comando %lu → HTTP %d\n", (unsigned long)ev.dato, codigo);
#endif
    return codigo >= 200 && codigo < 300;
  }
  return enviarAlerta(ev);
}

static void buscarPosicion() {
  // TODO(H3): paso 1 del ciclo. Salir antes si llega un evento a colaEventos
  //           (uxQueueMessagesWaiting(colaEventos) > 0).
}

// Paso 3: manda todo lo que hay en la cola. Lo que falla vuelve al final de la
// cola con el MISMO idLocal (reintento idempotente, CU05 2a). Un evento que
// falla no frena a los demás (p. ej. un ack no queda trabado detrás de una alerta).
static void enviarEventos() {
  UBaseType_t n = uxQueueMessagesWaiting(colaEventos);
  for (UBaseType_t i = 0; i < n; i++) {
    Evento ev;
    if (xQueueReceive(colaEventos, &ev, 0) != pdTRUE) break;
    if (enviarUno(ev)) continue;
    if (xQueueSendToBack(colaEventos, &ev, 0) != pdTRUE)
      Serial.println("[comunicacion] cola llena: se perdió un evento a reintentar");
  }
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
      if (codigo == 200 && procesarRespuesta(resp)) {
        // CU10: el ack sale en esta misma conexión, así la app anuncia
        // «El bastón está sonando» sin esperar otro ciclo de telemetría.
        vTaskDelay(pdMS_TO_TICKS(200));   // da tiempo a que comandos (núcleo 1) publique el ack
        enviarEventos();
      }
      modem::redReposo();
    }
#endif
    // Espera hasta el próximo ciclo, pero se despierta antes si llega un evento.
    // TODO(H3): reemplazar por una espera que mire colaEventos (p. ej. de a 500 ms).
    vTaskDelay(pdMS_TO_TICKS(ajustes::periodoTelemetriaMs()));
  }
}

}  // namespace comunicacion
