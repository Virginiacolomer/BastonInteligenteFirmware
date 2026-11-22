/* =============================================================================
 * baston.ino — Arranque del Bastón inteligente
 * -----------------------------------------------------------------------------
 * Dueño:     H1 · Conexión y armado general (integración)
 * Placa:     LilyGO T-SIM7080G-S3 · Arduino IDE · "ESP32S3 Dev Module"
 * Leer:      README.md (estructura, reglas y mapa de casos de uso)
 *
 * Acá NO va lógica: solo se inician los módulos y se los llama en orden.
 *   - setup(): iniciar() de cada módulo + crea la tarea de comunicación (núcleo 0).
 *   - loop():  actualizar() de cada módulo (núcleo 1). Sin delay().
 * Para probar por partes, apagar módulos en config.h (USAR_...).
 * ========================================================================== */
#include "config.h"
#include "estado.h"
#include "ajustes.h"
#include "energia.h"
#include "sensores.h"
#include "avisos.h"
#include "boton.h"
#include "obstaculos.h"
#include "panico.h"
#include "pulsera.h"
#include "bateria.h"
#include "comandos.h"
#include "comunicacion.h"
#include "consola.h"

void setup() {
  Serial.begin(115200);
  Serial.printf("\n[baston] firmware %s · contrato %s\n", VERSION_FIRMWARE, VERSION_CONTRATO);

  iniciarEstado();
  ajustes::iniciar();
#if USAR_ENERGIA
  if (!energia::iniciar()) Serial.println("[baston] ⚠ PMU no responde: revisar I2C");
#endif
#if USAR_SENSORES
  sensores::iniciar();
#endif
#if USAR_AVISOS
  avisos::iniciar();
#endif
#if USAR_BOTON
  boton::iniciar();
  panico::iniciar();
#endif
#if USAR_PULSERA
  pulsera::iniciar();
#endif
  obstaculos::iniciar();
  bateria::iniciar();
  comandos::iniciar();

  // Comunicación en el núcleo 0: puede bloquearse sin afectar los avisos.
  xTaskCreatePinnedToCore(comunicacion::tarea, "comunicacion", 12288, nullptr, 1, nullptr, 0);
}

void loop() {
  // Orden: primero leer entradas, después decidir, al final actuar.
  consola::actualizar();        // teclas de prueba del monitor serie
#if USAR_SENSORES
  sensores::actualizar();
  obstaculos::actualizar();     // CU01, CU02
#endif
#if USAR_BOTON
  boton::actualizar();
  panico::actualizar();         // CU03
#endif
#if USAR_PULSERA
  pulsera::actualizar();        // CU09
#endif
#if USAR_ENERGIA
  bateria::actualizar();        // CU11
#endif
  comandos::actualizar();       // CU10
#if USAR_AVISOS
  avisos::actualizar();         // motor y buzzer (siempre al final)
#endif
}
