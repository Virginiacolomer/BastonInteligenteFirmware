/* =============================================================================
 * modem.cpp — Implementación · Dueño: H3 · ver modem.h
 * Empezar por los ejemplos de LilyGO (ATDebug primero). Opción: usar TinyGSM.
 * ========================================================================== */
#include "config.h"
#include "modem.h"
#include "energia.h"

#define SerialAT Serial1

namespace modem {

// Envía un comando AT y espera `esperado` hasta timeoutMs. Devuelve true si llegó.
static bool at(const char *cmd, const char *esperado = "OK", uint32_t timeoutMs = 1000) {
  // TODO(H3): SerialAT.println(cmd); leer hasta encontrar `esperado` o "ERROR".
  (void)cmd; (void)esperado; (void)timeoutMs;
  return false;
}

bool encender() {
  energia::encenderModem(true);
  SerialAT.begin(115200, SERIAL_8N1, PIN_MODEM_RX, PIN_MODEM_TX);
  pinMode(PIN_MODEM_PWR, OUTPUT);
  // TODO(H3): PWRKEY HIGH ~1 s → LOW; esperar ~3 s; reintentar "AT" hasta OK.
  return at("AT");
}

bool   simLista() { return at("AT+CPIN?", "READY"); }
int8_t senal()    { /* TODO(H3): parsear +CSQ: <rssi>,<ber> */ return 99; }

bool gnssEncender() {
  redReposo();                       // ⚠ no pueden convivir
  energia::encenderGnss(true);
  return at("AT+CGNSPWR=1");
}
bool gnssLeer(Posicion &p) { /* TODO(H3): AT+CGNSINF → fix, lat, lon */ p.fix = false; return false; }
void gnssApagar()          { at("AT+CGNSPWR=0"); energia::encenderGnss(false); }

bool redConectar(uint32_t timeoutMs) {
  // TODO(H3): esperar AT+CEREG? = 0,1 o 0,5 hasta timeoutMs; activar datos con APN_OPERADORA.
  (void)timeoutMs; return false;
}
bool redConectada() { return false; /* TODO(H3) */ }
void redReposo()    { /* TODO(H3): desactivar datos / modo de bajo consumo */ }

int httpPost(const char *ruta, const String &json, String &respuesta) {
  // TODO(H3): HTTPS a API_HOST:API_PUERTO + API_BASE + ruta
  //   Encabezados: Content-Type: application/json · X-Device-Key: CLAVE_DISPOSITIVO
  //   Si TLS se traba en la semana 1, probar con HTTP SOLO en pruebas (cap. 13).
  (void)ruta; (void)json; respuesta = "";
  return -1;
}

}  // namespace modem
