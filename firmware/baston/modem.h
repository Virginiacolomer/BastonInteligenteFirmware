/* =============================================================================
 * modem.h — SIM7080G: red LTE-M/NB-IoT, GNSS y HTTPS por comandos AT
 * -----------------------------------------------------------------------------
 * Capa:          COMPONENTE (lo usa SOLO comunicacion.cpp, en el núcleo 0)
 * Dueño:         H3 · Botón de pánico (+ comunicación)
 * Casos de uso:  CU03, CU05, CU08, CU09, CU10, CU11, CU12 (todo lo que viaja)
 * Referencia:    cap. 7 del documento · ejemplos ATDebug / GPS / HTTPS de LilyGO
 *
 * ⚠ LIMITACIÓN DEL SIM7080G: el GNSS y la red NO funcionan al mismo tiempo.
 *   Antes de usar la red hay que apagar el GNSS (gnssApagar) y viceversa.
 * ⚠ Sin SMS: los avisos a contactos los manda el backend (CU04).
 * ⚠ Estas funciones PUEDEN TARDAR SEGUNDOS: nunca llamarlas desde el núcleo 1.
 *
 * Comandos AT de referencia:
 *   AT+CPIN?  → READY (SIM ok)        AT+CSQ   → señal
 *   AT+CEREG? → 0,1 / 0,5 (registrado) AT+CNACT → activar datos (APN)
 *   AT+CGNSPWR=1 / =0 → GNSS on/off    AT+CGNSINF → posición
 *   AT+SH... / AT+CSSLCFG → HTTP(S)    (ver manual de comandos AT del SIM7080G)
 * ========================================================================== */
#pragma once
#include <Arduino.h>

struct Posicion {
  bool   fix = false;
  double lat = 0, lon = 0;
};

namespace modem {
  bool    encender();                       // PWRKEY + espera "AT OK" (~3 s)
  bool    simLista();                       // AT+CPIN? → READY
  int8_t  senal();                          // AT+CSQ (0–31, 99 = sin señal)

  // GNSS (CU08)
  bool    gnssEncender();                   // apaga la red si estaba activa
  bool    gnssLeer(Posicion &p);            // true si hay fix
  void    gnssApagar();

  // Red y HTTPS (CU03/05/08/09/10/11/12)
  bool    redConectar(uint32_t timeoutMs);  // registro + datos con APN_OPERADORA
  bool    redConectada();
  void    redReposo();                      // deja la red en bajo consumo entre ciclos

  // Hace POST a API_HOST + ruta con el JSON y devuelve el código HTTP (o -1 si falla).
  // La respuesta queda en `respuesta` (puede venir vacía).
  int     httpPost(const char *ruta, const String &json, String &respuesta);
}
