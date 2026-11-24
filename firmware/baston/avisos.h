/* =============================================================================
 * avisos.h — Motor vibrador y buzzer: los patrones de aviso del cap. 8
 * -----------------------------------------------------------------------------
 * Capa:          COMPONENTE (lo usan obstaculos, panico, pulsera, bateria, comandos)
 * Dueño:         H4 · Detección de obstáculos
 * Casos de uso:  CU01, CU02, CU03, CU09, CU10, CU11 (todos los que suenan o vibran)
 * Historias:     HU01, HU02, HU03, HU06, HU07, HU10
 *
 * Qué hace:
 *   - Maneja el motor por PWM (IO10 → 2N2222) y el buzzer (IO9).
 *   - Reproduce PATRONES sin bloquear (todo con millis()).
 *   - Resuelve qué suena si hay varios pedidos a la vez (PRIORIDADES):
 *
 *     Buzzer (uno solo a la vez, gana el de mayor prioridad):
 *       1. PATRON_PANICO_ENVIADO / PATRON_ENVIO_FALLIDO / PATRON_PANICO_CONFIRMADO
 *       2. PATRON_CUENTA_PANICO (1 pitido por segundo)
 *       3. PATRON_PULSERA_SUELTA
 *       4. PATRON_SONAR_BASTON
 *       5. obstáculo del sensor ALTO (obstaculoBuzzer)
 *       6. PATRON_BATERIA_BAJA
 *     Motor: obstáculo del sensor BAJO (obstaculoMotor); PATRON_SONAR_BASTON y
 *       PATRON_PANICO_ENVIADO lo reemplazan mientras duran.
 *
 *   Aviso de obstáculos (CU02): pulsos de PULSO_AVISO_MS encendido cada
 *   `periodoMs`. obstaculos.cpp acorta el periodo a medida que se acerca el
 *   obstáculo; AVISO_CONTINUO = encendido sin cortes; 0 = apagado.
 *   El cambio de periodo se aplica sin cortar el pulso en curso.
 *
 * Tabla de patrones (cap. 8):
 *   CUENTA_PANICO ....... pitido corto cada 1 s
 *   PANICO_ENVIADO ...... pitido largo 1 s + vibración
 *   PANICO_CONFIRMADO ... 2 pitidos cortos (el backend respondió OK)
 *   ENVIO_FALLIDO ....... 3 pitidos graves
 *   PULSERA_SUELTA ...... pitido 100 ms cada 200 ms (mientras se pida)
 *   SONAR_BASTON ........ pitido + vibración 500 ms cada 1 s (mientras se pida)
 *   BATERIA_BAJA ........ 2 pitidos largos (una vez; bateria.cpp lo repite cada 10 min)
 * ========================================================================== */
#pragma once
#include <Arduino.h>

enum Patron : uint8_t {
  PATRON_NINGUNO,
  PATRON_CUENTA_PANICO,       // continuo  · CU03
  PATRON_PANICO_ENVIADO,      // una vez   · CU03
  PATRON_PANICO_CONFIRMADO,   // una vez   · CU03
  PATRON_ENVIO_FALLIDO,       // una vez   · CU03
  PATRON_PULSERA_SUELTA,      // continuo  · CU09
  PATRON_SONAR_BASTON,        // continuo  · CU10
  PATRON_BATERIA_BAJA,        // una vez   · CU11
  PATRON_CANTIDAD             // (no es un patrón: cantidad total)
};

constexpr uint16_t AVISO_CONTINUO = 1;    // periodo especial: encendido sin cortes

namespace avisos {
  void iniciar();                         // configura PWM del motor y el pin del buzzer
  void actualizar();                      // avanza los patrones activos (llamar en cada loop)

  // Obstáculos (CU02): 0 = apagado · AVISO_CONTINUO = sin cortes · otro = periodo en ms
  void obstaculoBuzzer(uint16_t periodoMs);   // sensor ALTO
  void obstaculoMotor(uint16_t periodoMs);    // sensor BAJO
  void activar(Patron p);                 // patrones CONTINUOS: suenan hasta desactivar()
  void desactivar(Patron p);
  void reproducir(Patron p);              // patrones de UNA VEZ: suenan y terminan solos
  void silenciarTodo();                   // apaga todo (p. ej. para pruebas)
}
