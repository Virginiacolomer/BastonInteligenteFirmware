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
 *       5. PATRON_OBSTACULO_ALTO
 *       6. PATRON_BATERIA_BAJA
 *     Motor: la vibración de obstáculo bajo (CU01) es CONTINUA y proporcional;
 *       los patrones de PATRON_SONAR_BASTON y PATRON_PANICO_ENVIADO la
 *       reemplazan mientras duran.
 *
 * Tabla de patrones (cap. 8):
 *   OBSTACULO_ALTO ...... pitido 120 ms cada 400 ms (mientras se pida)
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
  PATRON_OBSTACULO_ALTO,      // continuo  · CU02
  PATRON_CUENTA_PANICO,       // continuo  · CU03
  PATRON_PANICO_ENVIADO,      // una vez   · CU03
  PATRON_PANICO_CONFIRMADO,   // una vez   · CU03
  PATRON_ENVIO_FALLIDO,       // una vez   · CU03
  PATRON_PULSERA_SUELTA,      // continuo  · CU09
  PATRON_SONAR_BASTON,        // continuo  · CU10
  PATRON_BATERIA_BAJA         // una vez   · CU11
};

namespace avisos {
  void iniciar();                         // configura PWM del motor y el pin del buzzer
  void actualizar();                      // avanza los patrones activos (llamar en cada loop)

  void vibrarObstaculo(uint8_t pwm);      // CU01: 0 = apagado, 90–255 = intensidad continua
  void activar(Patron p);                 // patrones CONTINUOS: suenan hasta desactivar()
  void desactivar(Patron p);
  void reproducir(Patron p);              // patrones de UNA VEZ: suenan y terminan solos
  void silenciarTodo();                   // apaga todo (p. ej. para pruebas)
}
