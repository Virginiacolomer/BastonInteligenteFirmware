/* =============================================================================
 * energia.cpp — Implementación · Dueño: H1 · ver energia.h
 * Basarse en el ejemplo del repositorio LilyGo-T-SIM7080G (canales y voltajes).
 * ========================================================================== */
#include "config.h"
#include "energia.h"
// #include <XPowersLib.h>      // TODO(H1): descomentar al instalar la librería
// static XPowersPMU PMU;

namespace energia {

bool iniciar() {
  // TODO(H1): Wire.begin(PIN_PMU_SDA, PIN_PMU_SCL);
  //           PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, PIN_PMU_SDA, PIN_PMU_SCL);
  //           Configurar DC3 y BLDO1/BLDO2 con los voltajes del ejemplo de LilyGO.
  //           PMU.disableTSPinMeasure();   // sin NTC → si no, no carga
  //           PMU.enableBattVoltageMeasure();
  return true;
}

void encenderModem(bool on) { (void)on; /* TODO(H1): PMU.enableDC3() / disableDC3() */ }
void encenderGnss(bool on)  { (void)on; /* TODO(H1): PMU.enableBLDO2() / disableBLDO2() */ }

uint16_t bateriaMv()  { return 0;   /* TODO(H1): PMU.getBattVoltage() */ }
uint8_t  bateriaPct() { return 100; /* TODO(H1): PMU.getBatteryPercent() */ }
bool     cargando()   { return false; /* TODO(H1): PMU.isCharging() */ }

}  // namespace energia
