# Firmware del Bastón inteligente

Firmware de la placa **LilyGO T-SIM7080G-S3** (ESP32-S3 + módem SIM7080G con GPS).
Se programa con **Arduino IDE**. Este README es el mapa del código: léanlo antes de tocar nada.

> Referencias: *Documento de proyecto unificado* (PDF). Capítulos citados acá: cap. 5 (pines),
> cap. 7 (contrato con el backend), cap. 8 (patrones de aviso), cap. 10 (casos de uso), cap. 11 (HU).

---

## 1. Cómo abrirlo y compilarlo

1. Instalar en Arduino IDE el soporte **esp32 by Espressif Systems** (Gestor de placas).
2. Instalar las librerías (Gestor de librerías):
   - **XPowersLib** (Lewis He) — manejo del PMU AXP2101.
   - **ArduinoJson** (Benoit Blanchon, v7) — armar y leer los JSON del contrato.
   - **TinyGSM** (opcional) — si deciden usarla para el módem en vez de comandos AT a mano.
3. Completar `baston/secretos.h` con la URL del backend, la clave del dispositivo y el APN (ya viene creado con
   valores de ejemplo para que compile). **`secretos.h` no se sube al repositorio** (está en `.gitignore`);
   cada integrante que clone el repo copia `secretos.h.ejemplo` como `secretos.h`.
4. Abrir `baston/baston.ino`. Placa: **ESP32S3 Dev Module**. Opciones según el repo de LilyGO
   (Flash 16 MB, PSRAM *OPI*, *USB CDC On Boot: Enabled* para ver el monitor serie por USB).
5. Compilar y subir. Monitor serie a **115200**.

## 2. Estructura

```
firmware/
├── README.md               ← este archivo
├── .gitignore
└── baston/                 ← carpeta del sketch (Arduino compila todo lo que está acá)
    ├── baston.ino          ← arranque: setup() y loop(); crea la tarea de comunicación
    ├── config.h            ← CONTRATO FIJO: pines, valores por defecto, módulos activos
    ├── secretos.h.ejemplo  ← plantilla de URL y clave (copiar a secretos.h)
    ├── estado.h / .cpp     ← CONTRATO FIJO: datos compartidos + cola de eventos
    │
    │   CAPA DE COMPONENTES (cómo se habla con cada pieza; no deciden nada)
    ├── sensores.h / .cpp   ← HC-SR04 ×2
    ├── avisos.h / .cpp     ← motor + buzzer, patrones del cap. 8
    ├── boton.h / .cpp      ← pulsador antipánico (lectura pura)
    ├── energia.h / .cpp    ← PMU AXP2101, batería
    ├── modem.h / .cpp      ← SIM7080G: red, GNSS, HTTPS
    ├── consola.h / .cpp    ← teclas de prueba del monitor serie (s: pausa CSV · b: simula «sonar»)
    │
    │   CAPA DE FUNCIONES (casos de uso)
    ├── obstaculos.h / .cpp ← CU01, CU02
    ├── panico.h / .cpp     ← CU03
    ├── pulsera.h / .cpp    ← CU09
    ├── bateria.h / .cpp    ← CU11
    ├── comunicacion.h/.cpp ← CU08 + envío de eventos (CU03/05/09/11) + recepción
    ├── comandos.h / .cpp   ← CU10 (sonar el bastón)
    └── ajustes.h / .cpp    ← CU12 (ajustes desde la app, guardados en flash)
```

**Regla de dependencias:** los módulos de *función* usan a los de *componente*; nunca al revés.
Los módulos de función **no se llaman entre sí**: se comunican por `estado` y por la cola de eventos.

## 3. Mapa: casos de uso → archivos → dueños

| CU | Caso de uso | Archivos | HU | Dueño fase 1 | Paquete fase 2 |
|----|-------------|----------|----|--------------|----------------|
| CU01 | Detectar obstáculos | `obstaculos`, `sensores`, `avisos` | HU01 | H4 | I1 |
| CU02 | Avisar según la distancia | `obstaculos`, `sensores`, `avisos` | HU02 | H4 | I1 |
| CU03 | Activar alerta de pánico | `panico`, `boton`, `avisos` → `comunicacion` | HU03 | H3 | I2 |
| CU04 | Notificar a contactos | *(backend, no firmware)* | HU04 | S3 | I3 |
| CU05 | Registrar alerta | `comunicacion` (POST /eventos) | HU03 | H3 | I2 |
| CU06 | Mensaje personalizado | *(app + backend)* | HU11 | S2/S3 | I3 |
| CU07 | Gestionar contactos | *(app + backend)* | HU09 | S2 | I7 |
| CU08 | Publicar telemetría | `comunicacion`, `modem` | — | H3 | I4 |
| CU09 | Alertar pulsera suelta | `pulsera`, `avisos` → `comunicacion` | HU10 | H2 | I5 |
| CU10 | Sonar el bastón desde la app | `comandos`, `comunicacion`, `avisos`, `boton` | HU07 | H3 | I6 |
| CU11 | Avisar batería baja | `bateria`, `energia`, `avisos` → `comunicacion` | HU06 | H1 | I7 |
| CU12 | Configurar ajustes | `ajustes` (+ recepción en `comunicacion`) | HU09 | H1 | I6 |

| Integrante | Archivos que mantiene |
|---|---|
| **H1** Conexión y armado general | `baston.ino`, `config.h`, `estado.*`, `energia.*`, `bateria.*`, `ajustes.*` (integración) |
| **H2** Pulsera desconectada | `pulsera.*` |
| **H3** Botón de pánico (+ comunicación) | `boton.*`, `panico.*`, `modem.*`, `comunicacion.*`, `comandos.*` |
| **H4** Detección de obstáculos | `sensores.*`, `avisos.*`, `obstaculos.*` |
| **S1** Backend | revisa que `comunicacion.cpp` respete el contrato del cap. 7 |

## 4. Cómo corre el programa

```
            NÚCLEO 1  (loop de Arduino, cada ~5 ms, NUNCA se bloquea)
 ┌───────────────────────────────────────────────────────────────────┐
 │ sensores → obstaculos → avisos      (CU01, CU02: siempre activos) │
 │ boton → panico ───────┐                                           │
 │ pulsera ──────────────┼──► colaEventos ──┐                        │
 │ energia → bateria ────┘                  │                        │
 │ comandos (ejecuta "sonar")  ◄── estado ──┼──┐                     │
 └──────────────────────────────────────────┼──┼─────────────────────┘
                                            ▼  │
            NÚCLEO 0  (tarea de comunicación, puede tardar segundos)
 ┌───────────────────────────────────────────────────────────────────┐
 │ comunicacion: ciclo GPS → red → POST telemetría / eventos         │
 │               ← respuesta: comandos + ajustes → estado / ajustes  │
 │ modem: comandos AT al SIM7080G                                    │
 └───────────────────────────────────────────────────────────────────┘
```

## 5. Reglas del equipo

1. **Cada módulo tiene `iniciar()` y `actualizar()`.** `setup()` llama a los `iniciar()`, `loop()` a los `actualizar()`.
2. **Prohibido `delay()`** en el núcleo 1. Todo con `millis()`. Un `delay` frena el aviso de obstáculos (seguridad del usuario).
3. **Solo `comunicacion` y `modem` pueden bloquear**, porque corren en su propia tarea (núcleo 0).
4. **El acceso a `estado` va siempre con `BloqueoEstado`** (ver `estado.h`). Bloques cortos: copiar y soltar.
5. **Los módulos de función no se llaman entre sí.** Para avisar al backend: `publicarEvento(...)`.
6. **`config.h`, `estado.h` y el contrato JSON no se cambian sin avisar a todo el grupo.**
7. **Probar por partes:** en `config.h`, poner en `0` los módulos que no tengan conectados (`USAR_...`)
   y en `1` el `DEBUG_...` del propio módulo para ver sus datos en el monitor serie.
8. **Una rama por cambio**, revisión de otra persona antes de unir a `main`.
9. Los nombres de funciones y variables van en castellano y sin tildes (`distanciaBajo`, `iniciar`).
10. Cada `TODO(Hx)` indica quién tiene que completarlo.
