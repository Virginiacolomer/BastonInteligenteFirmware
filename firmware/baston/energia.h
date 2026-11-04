/* =============================================================================
 * energia.h — PMU AXP2101 de la placa: encendido de canales y lectura de batería
 * -----------------------------------------------------------------------------
 * Capa:          COMPONENTE
 * Dueño:         H1 · Conexión y armado general
 * Casos de uso:  CU11 Avisar batería baja (provee la lectura) · CU08 (la batería
 *                viaja en la telemetría) · necesario para que el MÓDEM funcione
 * Librería:      XPowersLib (Lewis He)
 *
 * Qué hace:
 *   - Inicia el PMU por I²C (SDA IO15, SCL IO7).
 *   - Enciende los canales que usa el bastón:
 *       DC3  → módem SIM7080G                (ver ejemplos de LilyGO)
 *       BLDO2 → antena GNSS activa           (lo prende/apaga modem.cpp)
 *       BLDO1 → adaptador de niveles ESP32↔módem  ⚠ NUNCA APAGAR
 *   - Desactiva la detección de temperatura (TS) para que cargue: la placa no
 *     trae NTC (indicación de LilyGO).
 *   - Lee tensión, porcentaje y si está cargando.
 *
 * Qué NO hace: decidir avisos de batería (bateria.cpp).
 * ========================================================================== */
#pragma once
#include <Arduino.h>

namespace energia {
  bool     iniciar();              // false si no encuentra el PMU (revisar I²C)
  void     encenderModem(bool on); // canal DC3
  void     encenderGnss(bool on);  // canal BLDO2 (antena activa)
  uint16_t bateriaMv();
  uint8_t  bateriaPct();           // 0–100
  bool     cargando();
}
