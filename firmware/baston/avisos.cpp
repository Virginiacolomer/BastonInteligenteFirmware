/* =============================================================================
 * avisos.cpp — Implementación · Dueño: H4 · ver tabla y prioridades en avisos.h
 * -----------------------------------------------------------------------------
 * Cada patrón es una lista de pasos {ms encendido, ms apagado}.
 *   - Continuos (repite = true): se repiten mientras estén activos.
 *   - De una vez (repite = false): suenan una vez desde reproducir() y terminan.
 * El buzzer reproduce SOLO el patrón de mayor prioridad que esté sonando; si
 * no hay ninguno por encima, los pulsos de obstáculo del sensor ALTO.
 * El motor vibra con SONAR_BASTON / PANICO_ENVIADO si están; si no, con los
 * pulsos de obstáculo del sensor BAJO (a PWM_MOTOR_MAX).
 * Buzzer ACTIVO: alcanza con HIGH/LOW (no necesita tono). Si su buzzer es
 * pasivo (no suena con HIGH fijo), cambiar BUZZER_ON por tone(PIN_BUZZER, 2700).
 * ========================================================================== */
#include "config.h"
#include "avisos.h"
#include "consola.h"
#include <initializer_list>

#define PWM_FREQ_HZ       20000   // 20 kHz: fuera del rango audible, el motor no "chilla"
#define PWM_BITS          8       // 0–255
#define PWM_CANAL         2       // canal 2 para no chocar con tone() (que usa canal 0 en core 2.x)

#define BUZZER_PASIVO     false   // el del bastón es ACTIVO (suena solo con 3,3 V). true solo si hace "clic" con HIGH
#define BUZZER_FREQ_HZ    2000    // frecuencia en Hz cuando BUZZER_PASIVO es true

namespace avisos {

struct Paso { uint16_t on, off; };
struct Def  { const Paso *pasos; uint8_t n; bool repite; bool motor; };

// --- Patrones del cap. 8 ---
static const Paso P_CUENTA[]      = {{80, 920}};                           // 1 por segundo
static const Paso P_ENVIADO[]     = {{1000, 0}};                           // largo 1 s
static const Paso P_CONFIRMADO[]  = {{100, 100}, {100, 0}};                // 2 cortos
static const Paso P_FALLIDO[]     = {{300, 150}, {300, 150}, {300, 0}};    // 3 largos
static const Paso P_PULSERA[]     = {{100, 100}};                          // rápido continuo
static const Paso P_SONAR[]       = {{500, 500}};                          // + vibración
static const Paso P_BATERIA[]     = {{600, 300}, {600, 0}};                // 2 largos

static const Def DEF[] = {
  /* NINGUNO           */ {nullptr, 0, false, false},
  /* CUENTA_PANICO     */ {P_CUENTA, 1, true, false},
  /* PANICO_ENVIADO    */ {P_ENVIADO, 1, false, true},
  /* PANICO_CONFIRMADO */ {P_CONFIRMADO, 2, false, false},
  /* ENVIO_FALLIDO     */ {P_FALLIDO, 3, false, false},
  /* PULSERA_SUELTA    */ {P_PULSERA, 1, true, false},
  /* SONAR_BASTON      */ {P_SONAR, 1, true, true},
  /* BATERIA_BAJA      */ {P_BATERIA, 2, false, false},
};

// Orden de prioridad del buzzer (el primero que esté sonando gana).
// Después de estos va el obstáculo del sensor alto y al final BATERIA_BAJA (ver actualizar()).
static const Patron PRIORIDAD_ALTA[] = {
  PATRON_PANICO_ENVIADO, PATRON_ENVIO_FALLIDO, PATRON_PANICO_CONFIRMADO,
  PATRON_CUENTA_PANICO, PATRON_PULSERA_SUELTA, PATRON_SONAR_BASTON
};
static_assert(sizeof(DEF) / sizeof(DEF[0]) == PATRON_CANTIDAD, "DEF tiene que tener una fila por patrón, en el orden del enum");

// Pulsos de obstáculo (CU02): buzzer = sensor alto · motor = sensor bajo.
struct Pulsos { uint16_t periodo; uint32_t inicio; };
static Pulsos pulsoBuzzer = {0, 0}, pulsoMotor = {0, 0};
static uint16_t activos = 0;                 // un bit por patrón continuo
static uint32_t inicioContinuo[PATRON_CANTIDAD] = {0};
static Patron   unaVez = PATRON_NINGUNO;
static uint32_t inicioUnaVez = 0;
// Nombre de cada patrón (se usa en actualizar() siempre, aunque solo se imprime con DEBUG_AVISOS).
static const char *const NOMBRE[] = {
  "ninguno", "cuenta_panico", "panico_enviado", "panico_confirmado",
  "envio_fallido", "pulsera_suelta", "sonar_baston", "bateria_baja"};
static_assert(sizeof(NOMBRE) / sizeof(NOMBRE[0]) == PATRON_CANTIDAD, "NOMBRE: una fila por patrón");
#if DEBUG_AVISOS
static const char *ultimoNombre = "ninguno";
// Prueba de banco (teclas del monitor serie, ver consola.h). Mientras dura,
// le gana a todo lo demás para que se note qué está fallando.
//   'z': buzzer solo, 1 s (si solo hace «clic», el buzzer es pasivo → BUZZER_PASIVO true)
//   'm': motor solo, 1 s a PWM_MOTOR_MAX
enum Prueba : uint8_t { PRUEBA_NADA, PRUEBA_BUZZER, PRUEBA_MOTOR };
static Prueba   prueba = PRUEBA_NADA;
static uint32_t pruebaDesdeMs = 0;
#endif

static void escribirMotor(uint8_t valor) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(PIN_MOTOR, valor);
#else
  ledcWrite(PWM_CANAL, valor);
#endif
}

static bool buzzerEncendido = false;

static void escribirBuzzer(bool encender) {
  if (encender == buzzerEncendido) return;  // evita reiniciar tone() en cada vuelta del loop
  buzzerEncendido = encender;

  if (BUZZER_PASIVO) {
    if (encender) tone(PIN_BUZZER, BUZZER_FREQ_HZ);
    else          noTone(PIN_BUZZER);
  } else {
    digitalWrite(PIN_BUZZER, encender ? HIGH : LOW);
  }
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

static void fijarPulsos(Pulsos &p, uint16_t periodo) {
  if (periodo != 0 && p.periodo == 0) p.inicio = millis();   // arranca con un pulso encendido
  p.periodo = periodo;
}

// ¿Está encendido ahora? Si el periodo se acorta, el próximo pulso llega antes.
static bool pulsoEncendido(Pulsos &p) {
  if (p.periodo == 0) return false;
  if (p.periodo == AVISO_CONTINUO) return true;
  uint32_t t = millis() - p.inicio;
  if (t >= p.periodo) { p.inicio = millis(); t = 0; }
  uint16_t on = PULSO_AVISO_MS < p.periodo ? PULSO_AVISO_MS : p.periodo / 2;
  return t < on;
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

void obstaculoBuzzer(uint16_t periodoMs) { fijarPulsos(pulsoBuzzer, periodoMs); }
void obstaculoMotor(uint16_t periodoMs)  { fijarPulsos(pulsoMotor,  periodoMs); }

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
  pulsoBuzzer.periodo = 0;
  pulsoMotor.periodo = 0;
  escribirBuzzer(false);
  escribirMotor(0);
}

void actualizar() {
  // 1. Buzzer: el patrón de mayor prioridad que esté sonando;
  //    si no hay, el obstáculo del sensor alto; si tampoco, batería baja.
  bool buzzer = false;
  const char *nombre = "ninguno";
  bool obstaculoOn = pulsoEncendido(pulsoBuzzer);   // se avanza siempre, suene o no
  bool elegido = false;
  for (Patron p : PRIORIDAD_ALTA) {
    bool on;
    if (sonando(p, on)) { buzzer = on; nombre = NOMBRE[p]; elegido = true; break; }
  }
  if (!elegido && pulsoBuzzer.periodo != 0) { buzzer = obstaculoOn; nombre = "obstaculo_alto"; elegido = true; }
  if (!elegido) {
    bool on;
    if (sonando(PATRON_BATERIA_BAJA, on)) { buzzer = on; nombre = NOMBRE[PATRON_BATERIA_BAJA]; }
  }
  // 2. Motor: patrones con vibración propia; si no, el obstáculo del sensor bajo
  uint8_t motor = pulsoEncendido(pulsoMotor) ? PWM_MOTOR_MAX : 0;
  for (Patron p : {PATRON_PANICO_ENVIADO, PATRON_SONAR_BASTON}) {
    bool on;
    if (DEF[p].motor && sonando(p, on)) { motor = on ? PWM_MOTOR_MAX : 0; break; }
  }

#if DEBUG_AVISOS
  // Prueba de banco: pisa lo calculado arriba mientras dura.
  if (consola::tecla('z')) { prueba = PRUEBA_BUZZER; pruebaDesdeMs = millis(); Serial.println("# prueba: buzzer 1 s"); }
  if (consola::tecla('m')) { prueba = PRUEBA_MOTOR;  pruebaDesdeMs = millis();
                             Serial.printf("# prueba: motor 1 s a PWM %u\n", PWM_MOTOR_MAX); }
  if (prueba != PRUEBA_NADA) {
    uint32_t t = millis() - pruebaDesdeMs;
    if (prueba == PRUEBA_BUZZER && t < 1000) { buzzer = true;  motor = 0; }
    else if (prueba == PRUEBA_MOTOR && t < 1000) { buzzer = false; motor = PWM_MOTOR_MAX; }
    else { prueba = PRUEBA_NADA; Serial.println("# prueba: fin"); }
  }
#endif

  escribirBuzzer(buzzer);
  escribirMotor(motor);

#if DEBUG_AVISOS
  if (nombre != ultimoNombre) { Serial.printf("# avisos: buzzer -> %s\n", nombre); ultimoNombre = nombre; }
#else
  (void)nombre;
#endif
}

}  // namespace avisos
