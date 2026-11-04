/* =============================================================================
 * estado.cpp — Implementación del estado compartido y la cola de eventos
 * Dueño: H1 · Ver la explicación en estado.h
 * ========================================================================== */
#include "config.h"
#include "estado.h"
#include "ajustes.h"

Estado        estado;
QueueHandle_t colaEventos = nullptr;
static SemaphoreHandle_t mutexEstado = nullptr;

void iniciarEstado() {
  mutexEstado = xSemaphoreCreateMutex();
  colaEventos = xQueueCreate(10, sizeof(Evento));   // hasta 10 eventos sin enviar
}

bool publicarEvento(TipoEvento tipo, uint16_t dato) {
  Evento e{tipo, ajustes::siguienteIdLocal(), millis(), dato};
  // No bloquea: si la cola está llena, se pierde el evento (y se avisa por serie).
  bool ok = xQueueSend(colaEventos, &e, 0) == pdTRUE;
  if (!ok) Serial.println("[estado] cola de eventos llena");
  return ok;
}

BloqueoEstado::BloqueoEstado()  { xSemaphoreTake(mutexEstado, portMAX_DELAY); }
BloqueoEstado::~BloqueoEstado() { xSemaphoreGive(mutexEstado); }
