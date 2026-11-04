/* =============================================================================
 * comandos.h — Ejecuta los comandos que llegan del backend
 * -----------------------------------------------------------------------------
 * Capa:          FUNCIÓN (caso de uso)
 * Dueño:         H3 · Paquete fase 2: I6
 * Casos de uso:  CU10 Hacer sonar el bastón desde la app («Encontrar mi bastón»)
 * Historias:     HU07 · Prueba P8
 *
 * Qué hace:
 *   - Si estado.sonarPendiente (lo pone comunicacion.cpp al leer la respuesta):
 *       avisos::activar(PATRON_SONAR_BASTON) durante DURACION_SONAR_S (30 s)
 *       y publicarEvento(EVENTO_ACK_COMANDO, id) para confirmar al backend.
 *   - Un toque corto al botón (boton::huboToqueCorto()) lo silencia antes.
 *   Comandos futuros (si se agregan al contrato) se suman acá.
 * ========================================================================== */
#pragma once

namespace comandos {
  void iniciar();
  void actualizar();
}
