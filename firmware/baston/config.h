/* =============================================================================
 * config.h — Tabla de pines, valores por defecto y módulos activos
 * -----------------------------------------------------------------------------
 * Capa:          configuración (CONTRATO FIJO: no cambiar sin avisar al grupo)
 * Dueño:         H1 · Conexión y armado general
 * Referencia:    Documento unificado, cap. 5 (pines) y cap. 8 (patrones)
 *
 * Qué hay acá:
 *   1. Módulos activos y depuración  → para probar por partes.
 *   2. Pines de los periféricos      → deben coincidir con el cableado real.
 *   3. Pines internos de la placa    → NO se cablean, vienen en la LilyGO.
 *   4. Valores por defecto           → los ajustables desde la app (CU12) son
 *                                       solo el valor inicial; el valor en uso
 *                                       vive en ajustes.cpp.
 * ========================================================================== */
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// 1. MÓDULOS ACTIVOS (1 = se usa, 0 = se saltea) y DEPURACIÓN por monitor serie
//    Ej.: para calibrar solo los sensores → USAR_MODEM 0, DEBUG_SENSORES 1.
// ---------------------------------------------------------------------------
// >>> CONFIGURACIÓN ACTUAL: PRUEBA DE SENSORES (solo sensores + motor + buzzer) <<<
//     Cuando se sumen los demás módulos, volver a poner en 1 los que correspondan.
#define USAR_SENSORES      1   // H4
#define USAR_AVISOS        1   // H4
#define USAR_BOTON         0   // H3
#define USAR_PULSERA       1   // H2
#define USAR_ENERGIA       0   // H1
#define USAR_MODEM         0   // H3 (0 = el bastón funciona sin red: útil en banco)

#define DEBUG_SENSORES     0   // CSV por el monitor serie: ms,bajo_cm,alto_cm,motor_pwm,aviso_alto
#define DEBUG_AVISOS       0
#define DEBUG_PANICO       0
#define DEBUG_PULSERA      1
#define DEBUG_ENERGIA      0
#define DEBUG_MODEM        0
#define DEBUG_COMUNICACION 0

#define VERSION_FIRMWARE   "0.1.0"
#define VERSION_CONTRATO   "v1"     // ruta /api/v1 del cap. 7

// ---------------------------------------------------------------------------
// 2. PINES DE LOS PERIFÉRICOS (cap. 5, tabla de pines)
// ---------------------------------------------------------------------------
#define PIN_TRIG_BAJO   16   // salida  · HC-SR04 inferior (tubo)
#define PIN_ECHO_BAJO   17   // entrada · por divisor 1k/2k (5 V → 3,3 V)
#define PIN_TRIG_ALTO   18   // salida  · HC-SR04 superior (mango, ~35°)
#define PIN_ECHO_ALTO    8   // entrada · por divisor 1k/2k
#define PIN_BUZZER       9   // salida
#define PIN_MOTOR       10   // salida PWM · base del 2N2222 por 1 kΩ
#define PIN_BOTON       11   // INPUT_PULLUP · presionado = LOW
#define PIN_PULSERA     12   // INPUT_PULLUP · enchufada = LOW, suelta = HIGH

// ---------------------------------------------------------------------------
// 3. PINES INTERNOS DE LA PLACA (no se cablean). Verificar contra utilities.h
//    del repositorio LilyGo-T-SIM7080G antes de la primera prueba del módem.
// ---------------------------------------------------------------------------
#define PIN_MODEM_RX     4   // como en los ejemplos de LilyGO: Serial1.begin(..., RX, TX)
#define PIN_MODEM_TX     5
#define PIN_MODEM_PWR   41   // PWRKEY del SIM7080G
#define PIN_MODEM_RI     3
#define PIN_MODEM_DTR   42
#define PIN_PMU_SDA     15   // I²C del AXP2101
#define PIN_PMU_SCL      7
#define PIN_PMU_IRQ      6
// No usar: IO35–37 (PSRAM), IO0/45/46 (arranque), TXD/RXD (monitor serie).

// ---------------------------------------------------------------------------
// 4. VALORES POR DEFECTO
// ---------------------------------------------------------------------------
// CU01 · obstáculo bajo
#define DIST_AVISO_BAJO_CM       150   // ajustable (CU12): 50–200
#define HISTERESIS_CM              8   // sigue avisando hasta 158 cm
#define KP_MOTOR                 1.1f  // u = KP·(dist_aviso − d)
#define PWM_MOTOR_MIN             90
#define PWM_MOTOR_MAX            255
// CU02 · obstáculo alto
#define DIST_AVISO_ALTO_CM       100   // ajustable (CU12): 50–150
// Medición
#define PERIODO_MEDICION_MS       30   // alterna bajo/alto cada 30 ms
#define TIMEOUT_ECO_US         25000   // sin eco → 400 cm (sin obstáculo)
#define DIST_SIN_ECO_CM          400
#define MUESTRAS_MEDIANA           5
// CU03 · pánico
#define TIEMPO_PANICO_MS        5000   // ajustable (CU12): 3–10 s
#define REINTENTO_EVENTO_MS    30000   // si no hay red, reintenta cada 30 s
// CU09 · pulsera
#define ANTIRREBOTE_PULSERA_MS  1000   // suelta > 1 s para disparar
#define AVISO_PULSERA_MS       30000   // sin reconectar en 30 s → evento
#define SONIDO_PULSERA_MAX_MS  60000   // deja de sonar a los 60 s
// CU10 · sonar el bastón
#define DURACION_SONAR_S          30
// CU11 · batería
#define BATERIA_BAJA_PCT          20
#define BATERIA_CRITICA_PCT       10
#define BATERIA_REARME_PCT        30   // al cargar por encima, se rearman los avisos
#define PERIODO_BATERIA_MS     10000
// CU08 · telemetría
#define TELEMETRIA_S              60   // ajustable (CU12): 30–300
#define TIMEOUT_GNSS_S            45
// Botón (genérico)
#define ANTIRREBOTE_BOTON_MS      30

#include "secretos.h"   // URL del backend y clave del dispositivo (no se sube al repo)
