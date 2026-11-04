/* =============================================================================
 * estado.h — Datos compartidos entre módulos y cola de eventos
 * -----------------------------------------------------------------------------
 * Capa:          núcleo (CONTRATO FIJO: no cambiar sin avisar al grupo)
 * Dueño:         H1 · Conexión y armado general
 * Casos de uso:  todos (es el "pizarrón" común)
 *
 * Por qué existe:
 *   El programa corre en dos núcleos (ver README, sección 4). Para que los
 *   módulos no se llamen entre sí, cada uno escribe lo que sabe en `estado`
 *   y lee lo que necesita. Los avisos al backend viajan por `colaEventos`.
 *
 * Quién escribe cada campo (y nadie más lo escribe):
 *   distBajo/distAlto ......... obstaculos   (núcleo 1)
 *   pulseraPuesta ............. pulsera      (núcleo 1)
 *   bateriaPct/mV/cargando .... bateria      (núcleo 1)
 *   gps* ...................... comunicacion (núcleo 0)
 *   senalCsq, conectado ....... comunicacion (núcleo 0)
 *   sonarPendiente ............ comunicacion → lo consume comandos
 *
 * Cómo se usa (SIEMPRE con el bloqueo, y bloques cortos):
 *   { BloqueoEstado b; estado.distBajo = d; }          // escribir
 *   Estado copia; { BloqueoEstado b; copia = estado; } // leer todo
 * ========================================================================== */
#pragma once
#include <Arduino.h>
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

struct Estado {
  // Obstáculos (CU01, CU02)
  uint16_t distBajo = DIST_SIN_ECO_CM;   // cm
  uint16_t distAlto = DIST_SIN_ECO_CM;   // cm
  // Pulsera (CU09)
  bool pulseraPuesta = true;
  // Batería (CU11)
  uint8_t  bateriaPct = 100;
  uint16_t bateriaMv  = 0;
  bool     cargando   = false;
  // Posición (CU08) — la última conocida, aunque sea vieja
  bool     gpsFix   = false;
  double   lat = 0, lon = 0;
  uint32_t gpsMs    = 0;      // millis() del último fix (para calcular la antigüedad)
  // Red (CU08)
  bool     conectado = false;
  int8_t   senalCsq  = 99;    // 0–31; 99 = sin señal
  // Comandos recibidos (CU10)
  bool     sonarPendiente = false;
  uint16_t idComandoSonar = 0;
};

// Tipos de evento que van a POST /api/v1/eventos (cap. 7)
enum TipoEvento : uint8_t {
  EVENTO_PANICO,          // CU03 → "panico"
  EVENTO_PULSERA,         // CU09 → "pulsera"
  EVENTO_BATERIA_BAJA,    // CU11 → "bateria_baja"
  EVENTO_ACK_COMANDO      // CU10 → POST /comandos/ack
};

struct Evento {
  TipoEvento tipo;
  uint32_t   idLocal;     // único por dispositivo: hace idempotente el reintento (CU05 2a)
  uint32_t   ms;          // millis() en que ocurrió
  uint16_t   dato;        // extra: % de batería, id de comando, etc.
};

extern Estado        estado;
extern QueueHandle_t colaEventos;

void iniciarEstado();                       // crea el mutex y la cola (lo llama setup)
bool publicarEvento(TipoEvento tipo, uint16_t dato = 0);  // desde cualquier módulo del núcleo 1

// Bloqueo del estado: se toma al crearse y se suelta al salir del bloque { }.
class BloqueoEstado {
 public:
  BloqueoEstado();
  ~BloqueoEstado();
};
