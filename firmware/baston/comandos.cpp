/* =============================================================================
 * comandos.cpp — Implementación · Dueño: H3 / I6 · CU10 · ver comandos.h
 * ========================================================================== */
#include "config.h"
#include "comandos.h"
#include "avisos.h"
#include "boton.h"
#include "estado.h"
#include "consola.h"

namespace comandos {

static bool     activo = false;
static uint32_t desdeMs = 0;
static uint32_t duracionMs = 0;
static bool     hayUltimoId = false;
static uint32_t ultimoId = 0;      // último comando "sonar" ejecutado (para no repetirlo)

static void empezar(uint32_t duracionS, const char *origen) {
  boton::huboToqueCorto();         // descarta un toque viejo: no tiene que silenciar lo que recién empieza
  avisos::activar(PATRON_SONAR_BASTON);
  activo = true;
  desdeMs = millis();
  duracionMs = duracionS * 1000UL;
  Serial.printf("[comandos] sonar %lu s (%s)\n", (unsigned long)duracionS, origen);
}

static void parar(const char *motivo) {
  avisos::desactivar(PATRON_SONAR_BASTON);
  activo = false;
  Serial.printf("[comandos] sonar terminado: %s\n", motivo);
}

void iniciar() {}

void actualizar() {
  // 1. ¿Llegó un comando "sonar" del backend?
  bool pendiente; uint32_t id; uint16_t durS;
  {
    BloqueoEstado b;
    pendiente = estado.sonarPendiente;
    id = estado.idComandoSonar;
    durS = estado.duracionSonarS;
    estado.sonarPendiente = false;
  }
  if (pendiente) {
    if (hayUltimoId && id == ultimoId) {
      Serial.printf("[comandos] sonar id %lu repetido: se ignora (el ack ya está en la cola)\n", (unsigned long)id);
    } else {
      hayUltimoId = true;
      ultimoId = id;
      empezar(durS, "app");
      publicarEvento(EVENTO_ACK_COMANDO, id);   // CU10 paso 4: confirmar al backend
    }
  }

#if DEBUG_COMANDOS
  // 2. Simulación sin app: tecla 'b' en el monitor serie.
  if (consola::tecla('b')) {
    if (activo) parar("tecla 'b'");
    else        empezar(DURACION_SONAR_S, "simulado con 'b', sin ack");
  }
#endif

  // 3. ¿Terminó? Por tiempo, o 4a: el usuario lo encontró y tocó el pulsador.
  if (activo) {
    if (millis() - desdeMs >= duracionMs) parar("pasó la duración");
    else if (boton::huboToqueCorto())     parar("toque al pulsador");
  }
}

bool sonando() { return activo; }

}  // namespace comandos
