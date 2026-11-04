/* =============================================================================
 * avisos.cpp — Implementación · Dueño: H4 · ver tabla y prioridades en avisos.h
 * ========================================================================== */
#include "config.h"
#include "avisos.h"

namespace avisos {

static uint8_t  pwmObstaculo = 0;
static uint16_t activos = 0;            // bit por patrón continuo activo
static Patron   unaVez = PATRON_NINGUNO;
static uint32_t inicioUnaVez = 0;

void iniciar() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  // TODO(H4): PWM del motor. En el core esp32 3.x: ledcAttach(PIN_MOTOR, 20000, 8);
  pinMode(PIN_MOTOR, OUTPUT);
}

void vibrarObstaculo(uint8_t pwm) { pwmObstaculo = pwm; }
void activar(Patron p)    { activos |=  (1u << p); }
void desactivar(Patron p) { activos &= ~(1u << p); }
void reproducir(Patron p) { unaVez = p; inicioUnaVez = millis(); }
void silenciarTodo()      { activos = 0; unaVez = PATRON_NINGUNO; pwmObstaculo = 0; }

void actualizar() {
  // TODO(H4):
  //  1. Elegir el patrón de buzzer de mayor prioridad (ver avisos.h): primero `unaVez`
  //     si todavía no terminó, después los continuos en `activos`.
  //  2. Calcular si el buzzer va ON u OFF según (millis() - inicio) % período del patrón.
  //  3. Motor: si hay SONAR_BASTON o PANICO_ENVIADO usar su patrón; si no, pwmObstaculo.
  //     Escribir con ledcWrite(PIN_MOTOR, valor).
  //  4. Cuando un patrón de una vez termina, volver unaVez a PATRON_NINGUNO.
}

}  // namespace avisos
