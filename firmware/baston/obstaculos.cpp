/* =============================================================================
 * obstaculos.cpp — Implementación · Dueño: H4 · CU01, CU02 · ver obstaculos.h
 * -----------------------------------------------------------------------------
 * Cada sensor maneja SU aviso, con la misma regla y distancias propias:
 *                         límite (intermitente)     continuo (fijo)
 *   ALTO → buzzer ....... 120 cm (fijo)             <  80 cm   (cabeza: fijo antes)
 *   BAJO → motor ........ 120 cm (ajuste CU12)      <  50 cm
 *   d ≥ límite ................ nada
 *   continuo ≤ d < límite ..... pulsos de PULSO_AVISO_MS; el periodo va de
 *                               PERIODO_AVISO_LEJOS_MS (en el límite) a
 *                               PERIODO_AVISO_CERCA_MS (justo antes del continuo)
 *   d < continuo .............. continuo
 * Histéresis: el aviso arranca al cruzar el límite y se apaga recién al superar
 * límite + HISTERESIS_CM; lo mismo para entrar y salir del continuo.
 * El periodo cambia de forma continua con la distancia (no hay tramos que titilen).
 * ⚠ Cambia la regla del cap. 8 (rev. D): ver CAMBIO_AVISOS_OBSTACULOS.txt, punto 8.
 * ========================================================================== */
#include "config.h"
#include "obstaculos.h"
#include "sensores.h"
#include "avisos.h"
#include "ajustes.h"
#include "estado.h"
#include "consola.h"

namespace obstaculos {

// Estado del aviso de UN sensor (para la histéresis).
struct Aviso {
  bool     activo   = false;   // hay obstáculo dentro del límite
  bool     continuo = false;   // está por debajo de la distancia de continuo
  uint16_t periodo  = 0;       // lo que se le pasa a avisos (0 / AVISO_CONTINUO / ms)
};

static Aviso avisoAlto, avisoBajo;   // alto → buzzer · bajo → motor
#if DEBUG_SENSORES
static uint32_t ultimoDebugMs = 0;
static bool     debugPausado = true;    // arranca en pausa: el CSV tapa los demás mensajes
static bool     informadoBajo = true, informadoAlto = true;   // último estado informado de cada sensor
#endif

// Calcula el periodo del aviso para la distancia d (con histéresis en los bordes).
//   limite:   desde acá empieza el aviso intermitente
//   continuo: por debajo de esto el aviso queda fijo
static uint16_t calcular(Aviso &a, uint16_t d, uint16_t limite, uint16_t continuo) {
  if (!a.activo) { if (d < limite) a.activo = true; }
  else if (d > limite + HISTERESIS_CM) a.activo = false;

  if (!a.continuo) { if (d < continuo) a.continuo = true; }
  else if (d > continuo + HISTERESIS_CM) a.continuo = false;

  if (!a.activo) return 0;
  if (a.continuo || limite <= continuo) return AVISO_CONTINUO;

  // Entre "continuo" y el límite: periodo proporcional a la distancia.
  uint16_t dd = d;
  if (dd < continuo) dd = continuo;   // saliendo del continuo (histéresis)
  if (dd > limite)   dd = limite;     // saliendo del aviso (histéresis)
  return PERIODO_AVISO_CERCA_MS +
         (uint32_t)(PERIODO_AVISO_LEJOS_MS - PERIODO_AVISO_CERCA_MS) * (dd - continuo) /
         (limite - continuo);
}

#if DEBUG_SENSORES
static const char *describir(uint16_t periodo, char *buf, size_t n) {
  if (periodo == 0)              return "nada";
  if (periodo == AVISO_CONTINUO) return "continuo";
  snprintf(buf, n, "1 cada %u ms", periodo);
  return buf;
}
#endif

void iniciar() {
#if DEBUG_SENSORES
  Serial.println("# CSV de sensores en pausa: 's' para verlo, 'r' para una sola lectura");
#endif
}

void actualizar() {
  uint16_t dBajo = sensores::distanciaBajo();
  uint16_t dAlto = sensores::distanciaAlto();
  uint16_t limiteBajo = ajustes::distAvisoBajo();

  avisoAlto.periodo = calcular(avisoAlto, dAlto, DIST_AVISO_ALTO_CM, DIST_CONTINUO_ALTO_CM);
  avisoBajo.periodo = calcular(avisoBajo, dBajo, limiteBajo,         DIST_CONTINUO_BAJO_CM);
  avisos::obstaculoBuzzer(avisoAlto.periodo);
  avisos::obstaculoMotor(avisoBajo.periodo);

  { BloqueoEstado b; estado.distBajo = dBajo; estado.distAlto = dAlto; }

#if DEBUG_SENSORES
  // La letra 's' en el monitor serie activa / pausa el CSV.
  // Columnas motor_ms y buzzer_ms: 0 = nada · 1 = continuo · otro = periodo de los pulsos.
  if (consola::tecla('s')) {
    debugPausado = !debugPausado;
    Serial.println(debugPausado ? "# CSV en pausa ('s' para seguir)" : "ms,bajo_cm,alto_cm,motor_ms,buzzer_ms");
  }
  // La letra 'r' muestra UNA lectura completa (filtrada y cruda), sin el CSV.
  if (consola::tecla('r')) {
    char b1[20], b2[20];
    Serial.printf("# bajo %u cm (crudo %u, %s) · alto %u cm (crudo %u, %s)\n",
                  dBajo, sensores::crudaBajo(), sensores::respondeBajo() ? "responde" : "NO responde",
                  dAlto, sensores::crudaAlto(), sensores::respondeAlto() ? "responde" : "NO responde");
    Serial.printf("#   motor (bajo): %s · buzzer (alto): %s\n",
                  describir(avisoBajo.periodo, b1, sizeof b1), describir(avisoAlto.periodo, b2, sizeof b2));
  }
  // CU01 1a: avisa una vez cuando un sensor deja de responder o vuelve.
  bool rb = sensores::respondeBajo(), ra = sensores::respondeAlto();
  if (rb != informadoBajo) { informadoBajo = rb; Serial.println(rb ? "# sensor BAJO responde de nuevo" : "# sensor BAJO no responde: revisar cable (IO16/IO17)"); }
  if (ra != informadoAlto) { informadoAlto = ra; Serial.println(ra ? "# sensor ALTO responde de nuevo" : "# sensor ALTO no responde: revisar cable (IO18/IO8)"); }
  // CSV 10 veces por segundo: copiar del monitor serie a una planilla (P1–P4).
  if (!debugPausado && millis() - ultimoDebugMs >= 100) {
    ultimoDebugMs = millis();
    Serial.printf("%lu,%u,%u,%u,%u\n", (unsigned long)millis(), dBajo, dAlto, avisoBajo.periodo, avisoAlto.periodo);
  }
#endif
}

}  // namespace obstaculos
