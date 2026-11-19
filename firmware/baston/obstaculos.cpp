/* =============================================================================
 * obstaculos.cpp — Implementación · Dueño: H4 · CU01, CU02 · ver obstaculos.h
 * -----------------------------------------------------------------------------
 * CU01 (bajo):  aviso ON si d < límite; OFF recién si d > límite + HISTERESIS_CM
 *               intensidad: u = PWM_MOTOR_MIN + KP_MOTOR·(límite − d), tope 255
 *               (con límite 150 y KP 1,1: 90 a 150 cm → 255 pegado al objeto)
 * CU02 (alto):  pitido mientras d < límite alto (con 5 cm de histéresis)
 * ========================================================================== */
#include "config.h"
#include "obstaculos.h"
#include "sensores.h"
#include "avisos.h"
#include "ajustes.h"
#include "estado.h"

#define HISTERESIS_ALTO_CM 5

namespace obstaculos {

static bool    avisoBajoActivo = false;
static bool    avisoAltoActivo = false;
static uint8_t pwmActual = 0;
#if DEBUG_SENSORES
static uint32_t ultimoDebugMs = 0;
#endif

void iniciar() {
#if DEBUG_SENSORES
  Serial.println("ms,bajo_cm,alto_cm,motor_pwm,aviso_alto");
#endif
}

void actualizar() {
  uint16_t dBajo = sensores::distanciaBajo();
  uint16_t dAlto = sensores::distanciaAlto();

  // --- CU01 · obstáculo bajo, con histéresis ---
  uint16_t limBajo = ajustes::distAvisoBajo();
  if (!avisoBajoActivo && dBajo < limBajo) avisoBajoActivo = true;
  else if (avisoBajoActivo && dBajo > limBajo + HISTERESIS_CM) avisoBajoActivo = false;

  pwmActual = 0;
  if (avisoBajoActivo) {
    float extra = KP_MOTOR * (float)((int)limBajo - (int)dBajo);
    if (extra < 0) extra = 0;                         // zona de histéresis: intensidad mínima
    int u = PWM_MOTOR_MIN + (int)extra;
    if (u > PWM_MOTOR_MAX) u = PWM_MOTOR_MAX;
    pwmActual = (uint8_t)u;
  }
  avisos::vibrarObstaculo(pwmActual);

  // --- CU02 · obstáculo alto ---
  uint16_t limAlto = ajustes::distAvisoAlto();
  if (!avisoAltoActivo && dAlto < limAlto) { avisoAltoActivo = true; avisos::activar(PATRON_OBSTACULO_ALTO); }
  else if (avisoAltoActivo && dAlto > limAlto + HISTERESIS_ALTO_CM) { avisoAltoActivo = false; avisos::desactivar(PATRON_OBSTACULO_ALTO); }

  { BloqueoEstado b; estado.distBajo = dBajo; estado.distAlto = dAlto; }

#if DEBUG_SENSORES
  // CSV 10 veces por segundo: copiar del monitor serie a una planilla para la prueba de cobertura.
  if (millis() - ultimoDebugMs >= 100) {
    ultimoDebugMs = millis();
    Serial.printf("%lu,%u,%u,%u,%u\n", (unsigned long)millis(), dBajo, dAlto, pwmActual, avisoAltoActivo ? 1 : 0);
  }
#endif
}

}  // namespace obstaculos
