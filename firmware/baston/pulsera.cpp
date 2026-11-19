/* =============================================================================
 * pulsera.cpp — Implementación · Dueño: H2 · CU09 · ver pulsera.h
 * ========================================================================== */
#include "config.h"
#include "pulsera.h"
#include "avisos.h"
#include "ajustes.h"
#include "estado.h"

namespace pulsera {

enum Fase : uint8_t { PUESTA, SUELTA, AVISADA };
static Fase     fase = PUESTA;
static uint32_t desconectadaDesdeMs = 0;
static bool     sonando = false;
#if DEBUG_PULSERA
static Fase     ultimaFaseDebug = PUESTA;
#endif

void iniciar() {
  pinMode(PIN_PULSERA, INPUT_PULLUP);
  fase = PUESTA;
  desconectadaDesdeMs = 0;
  sonando = false;
}

void actualizar() {
  bool pinSuelta = (digitalRead(PIN_PULSERA) == HIGH); // HIGH = desconectada
  uint32_t ahora = millis();

  if (!pinSuelta) {
    // -------------------------------------------------------------------------
    // La pulsera está conectada (IO12 a GND -> LOW)
    // -------------------------------------------------------------------------
    desconectadaDesdeMs = 0;
    if (fase != PUESTA) {
      fase = PUESTA;
      if (sonando) {
        avisos::desactivar(PATRON_PULSERA_SUELTA);
        sonando = false;
      }
    }
  } else {
    // -------------------------------------------------------------------------
    // La pulsera está desconectada (circuito abierto -> HIGH)
    // -------------------------------------------------------------------------
    if (desconectadaDesdeMs == 0) {
      desconectadaDesdeMs = ahora;
    }

    uint32_t tiempoDesconectada = ahora - desconectadaDesdeMs;

    // 1. Confirmar desconexión con antirrebote (1 segundo)
    if (fase == PUESTA) {
      if (tiempoDesconectada >= ANTIRREBOTE_PULSERA_MS) {
        fase = SUELTA;
        if (!ajustes::modoCasa()) {
          avisos::activar(PATRON_PULSERA_SUELTA);
          sonando = true;
        }
      }
    }
    // 2. Si sigue suelta 30 segundos -> disparar evento al backend (CU09 / CU05)
    else if (fase == SUELTA) {
      if (tiempoDesconectada >= AVISO_PULSERA_MS) {
        fase = AVISADA;
        if (!ajustes::modoCasa()) {
          publicarEvento(EVENTO_PULSERA);
        }
      }
    }

    // 3. Pasados los 60 segundos de sonido máximo, silenciar
    if (sonando && (tiempoDesconectada >= SONIDO_PULSERA_MAX_MS || ajustes::modoCasa())) {
      avisos::desactivar(PATRON_PULSERA_SUELTA);
      sonando = false;
    }
  }

  // Copia el estado para telemetría hacia el núcleo 0
  {
    BloqueoEstado b;
    estado.pulseraPuesta = (fase == PUESTA);
  }

#if DEBUG_PULSERA
  if (fase != ultimaFaseDebug) {
    const char *nombres[] = {"PUESTA", "SUELTA", "AVISADA"};
    Serial.printf("[pulsera] estado -> %s (sonando=%d)\n", nombres[fase], sonando ? 1 : 0);
    ultimaFaseDebug = fase;
  }
#endif
}

bool puesta() {
  return fase == PUESTA;
}

}  // namespace pulsera
