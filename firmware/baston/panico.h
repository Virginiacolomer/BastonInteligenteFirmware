/* =============================================================================
 * panico.h — Alerta de pánico desde el botón del bastón
 * -----------------------------------------------------------------------------
 * Capa:          FUNCIÓN (caso de uso)
 * Dueño:         H3 · Botón de pánico            · Paquete fase 2: I2
 * Casos de uso:  CU03 Activar alerta de pánico  (incluye CU05 y CU04, que
 *                resuelven comunicacion.cpp y el backend)
 * Historias:     HU03 · Prueba P5 (soltar a los 2–3 s NO envía nada)
 *
 * Máquina de estados:
 *   REPOSO ──(botón presionado)──► CONTANDO: avisos::activar(PATRON_CUENTA_PANICO)
 *   CONTANDO ──(suelta antes de tiempo)──► REPOSO (se cancela, no se envía nada)
 *   CONTANDO ──(llega a ajustes::tiempoPanicoMs())──► ENVIADO:
 *        avisos::reproducir(PATRON_PANICO_ENVIADO); publicarEvento(EVENTO_PANICO)
 *   ENVIADO ──(suelta el botón)──► REPOSO
 *
 * La confirmación (2 pitidos) o el fallo (3 pitidos graves) los dispara
 * comunicacion.cpp cuando el backend responde (o no).
 * El pánico desde la APP no pasa por acá: llega directo al backend.
 * Durante todo el caso, CU01 y CU02 siguen funcionando.
 * ========================================================================== */
#pragma once

namespace panico {
  void iniciar();
  void actualizar();
}
