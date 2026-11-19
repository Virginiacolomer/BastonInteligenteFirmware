/* =============================================================================
 * avisos.cpp — Implementación · Dueño: H4 · ver tabla y prioridades en avisos.h
 * -----------------------------------------------------------------------------
 * Cada patrón es una lista de pasos {ms encendido, ms apagado}.
 *   - Continuos (repite = true): se repiten mientras estén activos.
 *   - De una vez (repite = false): suenan una vez desde reproducir() y terminan.
 * El buzzer reproduce SOLO el patrón de mayor prioridad que esté sonando.
 * El motor vibra con SONAR_BASTON / PANICO_ENVIADO si están; si no, con la
 * intensidad de obstáculo bajo (CU01).
 * Buzzer ACTIVO: alcanza con HIGH/LOW (no necesita tono). Si su buzzer es
 * pasivo (no suena con HIGH fijo), cambiar BUZZER_ON por tone(PIN_BUZZER, 2700).
 * ========================================================================== */
#include "config.h"
#include "avisos.h"
#include <initializer_list>

#define PWM_FREQ_HZ  20000   // 20 kHz: fuera del rango audible, el motor no "chilla"
#define PWM_BITS     8       // 0–255
#define PWM_CANAL    0       // solo para el core esp32 2.x

namespace avisos {

struct Paso { uint16_t on, off; };
struct Def  { const Paso *pasos; uint8_t n; bool repite; bool motor; };

// --- Patrones del cap. 8 ---
static const Paso P_ALTO[]        = {{120, 280}};                          // cada 400 ms
static const Paso P_CUENTA[]      = {{80, 920}};                           // 1 por segundo
static const Paso P_ENVIADO[]     = {{1000, 0}};                           // largo 1 s
static const Paso P_CONFIRMADO[]  = {{100, 100}, {100, 0}};                // 2 cortos
static const Paso P_FALLIDO[]     = {{300, 150}, {300, 150}, {300, 0}};    // 3 largos
static const Paso P_PULSERA[]     = {{100, 100}};                          // rápido continuo
static const Paso P_SONAR[]       = {{500, 500}};                          // + vibración
static const Paso P_BATERIA[]     = {{600, 300}, {600, 0}};                // 2 largos

static const Def DEF[] = {
  /* NINGUNO           */ {nullptr, 0, false, false},
  /* OBSTACULO_ALTO    */ {P_ALTO, 1, true, false},
  /* CUENTA_PANICO     */ {P_CUENTA, 1, true, false},
  /* PANICO_ENVIADO    */ {P_ENVIADO, 1, false, true},
  /* PANICO_CONFIRMADO */ {P_CONFIRMADO, 2, false, false},
  /* ENVIO_FALLIDO     */ {P_FALLIDO, 3, false, false},
  /* PULSERA_SUELTA    */ {P_PULSERA, 1, true, false},
  /* SONAR_BASTON      */ {P_SONAR, 1, true, true},
  /* BATERIA_BAJA      */ {P_BATERIA, 2, false, false},
};

// Orden de prioridad del buzzer (el primero que esté sonando gana)
static const Patron PRIORIDAD[] = {
  PATRON_PANICO_ENVIADO, PATRON_ENVIO_FALLIDO, PATRON_PANICO_CONFIRMADO,
  PATRON_CUENTA_PANICO, PATRON_PULSERA_SUELTA, PATRON_SONAR_BASTON,
  PATRON_OBSTACULO_ALTO, PATRON_BATERIA_BAJA
};

static uint8_t  pwmObstaculo = 0;
static uint16_t activos = 0;                 // un bit por patrón continuo
static uint32_t inicioContinuo[9] = {0};
static Patron   unaVez = PATRON_NINGUNO;
static uint32_t inicioUnaVez = 0;
#if DEBUG_AVISOS
static Patron   ultimoDebug = PATRON_NINGUNO;
#endif

static void escribirMotor(uint8_t valor) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(PIN_MOTOR, valor);
#else
  ledcWrite(PWM_CANAL, valor);
#endif
}

// Devuelve true si el patrón está encendido en el instante t (ms desde su inicio).
// En los de una vez, `terminado` indica que ya pasaron todos los pasos.
static bool encendidoEn(const Def &d, uint32_t t, bool &terminado) {
  uint32_t total = 0;
  for (uint8_t i = 0; i < d.n; i++) total += d.pasos[i].on + d.pasos[i].off;
  terminado = false;
  if (total == 0) { terminado = true; return false; }
  if (d.repite) t %= total;
  else if (t >= total) { terminado = true; return false; }
  for (uint8_t i = 0; i < d.n; i++) {
    if (t < d.pasos[i].on) return true;
    t -= d.pasos[i].on;
    if (t < d.pasos[i].off) return false;
    t -= d.pasos[i].off;
  }
  return false;
}

// ¿Está sonando el patrón p ahora? (activo si es continuo, o en curso si es de una vez)
static bool sonando(Patron p, bool &on) {
  const Def &d = DEF[p];
  bool fin;
  if (d.repite) {
    if (!(activos & (1u << p))) return false;
    on = encendidoEn(d, millis() - inicioContinuo[p], fin);
    return true;
  }
  if (unaVez != p) return false;
  on = encendidoEn(d, millis() - inicioUnaVez, fin);
  if (fin) { unaVez = PATRON_NINGUNO; return false; }
  return true;
}

void iniciar() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(PIN_MOTOR, PWM_FREQ_HZ, PWM_BITS);
#else
  ledcSetup(PWM_CANAL, PWM_FREQ_HZ, PWM_BITS);
  ledcAttachPin(PIN_MOTOR, PWM_CANAL);
#endif
  escribirMotor(0);
}

void vibrarObstaculo(uint8_t pwm) { pwmObstaculo = pwm; }

void activar(Patron p) {
  if (!DEF[p].repite) { reproducir(p); return; }
  if (!(activos & (1u << p))) inicioContinuo[p] = millis();   // arranca desde el principio
  activos |= (1u << p);
}

void desactivar(Patron p) { activos &= ~(1u << p); }

void reproducir(Patron p) {
  if (DEF[p].repite) { activar(p); return; }
  unaVez = p;
  inicioUnaVez = millis();
}

void silenciarTodo() {
  activos = 0;
  unaVez = PATRON_NINGUNO;
  pwmObstaculo = 0;
  digitalWrite(PIN_BUZZER, LOW);
  escribirMotor(0);
}

void actualizar() {
  // 1. Buzzer: el patrón de mayor prioridad que esté sonando
  bool buzzer = false;
  Patron elegido = PATRON_NINGUNO;
  for (Patron p : PRIORIDAD) {
    bool on;
    if (sonando(p, on)) { buzzer = on; elegido = p; break; }
  }
  digitalWrite(PIN_BUZZER, buzzer ? HIGH : LOW);

  // 2. Motor: patrones con vibración propia, si no la de obstáculo bajo
  uint8_t motor = pwmObstaculo;
  for (Patron p : {PATRON_PANICO_ENVIADO, PATRON_SONAR_BASTON}) {
    bool on;
    if (DEF[p].motor && sonando(p, on)) { motor = on ? PWM_MOTOR_MAX : 0; break; }
  }
  escribirMotor(motor);

#if DEBUG_AVISOS
  if (elegido != ultimoDebug) { Serial.printf("[avisos] buzzer: patron %u\n", elegido); ultimoDebug = elegido; }
#else
  (void)elegido;
#endif
}

}  // namespace avisos
