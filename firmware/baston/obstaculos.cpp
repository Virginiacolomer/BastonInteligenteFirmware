/* =============================================================================
 * obstaculos.cpp — Implementación · Dueño: H4 · CU01, CU02 · ver obstaculos.h
 * Coincide con el cap. 8 del documento unificado (rev. D).
 * -----------------------------------------------------------------------------
 * d = la MENOR distancia de los dos sensores (bajo y alto). Tramos:
 *   CERCA  d < 50 cm .............. buzzer continuo   (PATRON_OBSTACULO_CERCA)
 *   MEDIO  50 cm ≤ d < 1 m ........ pitido 120/400 ms (PATRON_OBSTACULO_MEDIO)
 *   VIBRA  1 m ≤ d < límite ....... motor proporcional: PWM_MOTOR_MIN en el
 *                                   límite → PWM_MOTOR_MAX a 1 m
 *   NADA   d ≥ límite ............. sin aviso
 * límite = ajustes::distAvisoBajo() (150 cm por defecto, 50–200 desde la app;
 * si es ≤ 100 cm el tramo de vibración desaparece).
 * Histéresis: a un tramo más CERCANO se pasa enseguida; a uno más LEJANO recién
 * cuando d supera el borde del tramo actual + HISTERESIS_CM (evita que titile).
 * ========================================================================== */
#include "config.h"
#include "obstaculos.h"
#include "sensores.h"
#include "avisos.h"
#include "ajustes.h"
#include "estado.h"
#include "consola.h"

namespace obstaculos {

enum Zona : uint8_t { ZONA_CERCA, ZONA_MEDIO, ZONA_VIBRA, ZONA_NADA };

static Zona    zona = ZONA_NADA;
static uint8_t pwmActual = 0;
#if DEBUG_SENSORES
static uint32_t ultimoDebugMs = 0;
static bool     debugPausado = true;    // arranca en pausa: el CSV tapa los demás mensajes
static bool     avisoBajo = true, avisoAlto = true;   // último estado informado de cada sensor
#endif

static Zona zonaDe(uint16_t d, uint16_t limite) {
  if (d < DIST_CERCA_CM) return ZONA_CERCA;
  if (d < DIST_MEDIO_CM) return ZONA_MEDIO;
  if (d < limite)        return ZONA_VIBRA;
  return ZONA_NADA;
}

// Borde lejano del tramo z (a partir de ahí, + histéresis, se sale del tramo).
static uint16_t bordeLejano(Zona z, uint16_t limite) {
  switch (z) {
    case ZONA_CERCA: return DIST_CERCA_CM;
    case ZONA_MEDIO: return DIST_MEDIO_CM;
    default:         return limite;
  }
}

static Patron patronDe(Zona z) {
  if (z == ZONA_CERCA) return PATRON_OBSTACULO_CERCA;
  if (z == ZONA_MEDIO) return PATRON_OBSTACULO_MEDIO;
  return PATRON_NINGUNO;
}

void iniciar() {
#if DEBUG_SENSORES
  Serial.println("# CSV de sensores en pausa: 's' para verlo, 'r' para una sola lectura");
#endif
}

void actualizar() {
  uint16_t dBajo = sensores::distanciaBajo();
  uint16_t dAlto = sensores::distanciaAlto();
  uint16_t d = dBajo < dAlto ? dBajo : dAlto;
  uint16_t limite = ajustes::distAvisoBajo();

  // --- Tramo, con histéresis solo al alejarse ---
  Zona nueva = zonaDe(d, limite);
  Zona anterior = zona;
  if (nueva < zona) zona = nueva;
  else if (nueva > zona && d > bordeLejano(zona, limite) + HISTERESIS_CM) zona = nueva;

  // --- Buzzer: cambia el patrón solo cuando cambia el tramo ---
  if (zona != anterior) {
    Patron viejo = patronDe(anterior), actual = patronDe(zona);
    if (viejo  != PATRON_NINGUNO) avisos::desactivar(viejo);
    if (actual != PATRON_NINGUNO) avisos::activar(actual);
  }

  // --- Motor: proporcional solo en el tramo de vibración ---
  pwmActual = 0;
  if (zona == ZONA_VIBRA && limite > DIST_MEDIO_CM) {
    int u = PWM_MOTOR_MIN;
    if (d < limite) {   // d ≥ límite = zona de histéresis: intensidad mínima
      int dd = d < DIST_MEDIO_CM ? DIST_MEDIO_CM : d;
      u += (PWM_MOTOR_MAX - PWM_MOTOR_MIN) * (int)(limite - dd) / (int)(limite - DIST_MEDIO_CM);
    }
    if (u > PWM_MOTOR_MAX) u = PWM_MOTOR_MAX;
    pwmActual = (uint8_t)u;
  }
  avisos::vibrarObstaculo(pwmActual);

  { BloqueoEstado b; estado.distBajo = dBajo; estado.distAlto = dAlto; }

#if DEBUG_SENSORES
  // La letra 's' en el monitor serie pausa y reanuda el CSV.
  if (consola::tecla('s')) {
    debugPausado = !debugPausado;
    Serial.println(debugPausado ? "# CSV en pausa ('s' para seguir)" : "ms,bajo_cm,alto_cm,motor_pwm,zona");
  }
  // La letra 'r' muestra UNA lectura completa (filtrada y cruda), sin el CSV.
  if (consola::tecla('r')) {
    static const char *const ZONA[] = {"cerca (<50 cm: buzzer continuo)", "medio (50-100 cm: pitidos)",
                                       "vibra (100 cm al limite)", "nada"};
    Serial.printf("# bajo %u cm (crudo %u, %s) · alto %u cm (crudo %u, %s)\n",
                  dBajo, sensores::crudaBajo(), sensores::respondeBajo() ? "responde" : "NO responde",
                  dAlto, sensores::crudaAlto(), sensores::respondeAlto() ? "responde" : "NO responde");
    Serial.printf("#   zona %s · motor PWM %u\n", ZONA[zona], pwmActual);
  }
  // CU01 1a: avisa una vez cuando un sensor deja de responder o vuelve.
  bool rb = sensores::respondeBajo(), ra = sensores::respondeAlto();
  if (rb != avisoBajo) { avisoBajo = rb; Serial.println(rb ? "# sensor BAJO responde de nuevo" : "# sensor BAJO no responde: revisar cable (IO16/IO17)"); }
  if (ra != avisoAlto) { avisoAlto = ra; Serial.println(ra ? "# sensor ALTO responde de nuevo" : "# sensor ALTO no responde: revisar cable (IO18/IO8)"); }
  // CSV 10 veces por segundo: copiar del monitor serie a una planilla (P1–P4).
  if (!debugPausado && millis() - ultimoDebugMs >= 100) {
    ultimoDebugMs = millis();
    Serial.printf("%lu,%u,%u,%u,%u\n", (unsigned long)millis(), dBajo, dAlto, pwmActual, (unsigned)zona);
  }
#endif
}

}  // namespace obstaculos
