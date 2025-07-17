# ⚡ Detector de Sobretensión con ESP32 y Sensor ZMPT101B

Este proyecto de **código abierto** permite implementar un **sistema de protección contra sobretensiones** usando un microcontrolador **ESP32** junto con un **sensor ZMPT101B**. El sistema mide el voltaje en tiempo real y activa un **relé** para cortar el suministro eléctrico si el voltaje supera un umbral definido.

> ✅ Este repositorio incluye el código `.ino` completo del ESP32.

---

## 📦 Materiales Requeridos

- ESP32 (cualquier modelo con ADC funcional)
- Sensor de voltaje ZMPT101B
- Módulo de relé
- Protoboard
- Cables Dupont
- Computadora con Arduino IDE
- Broker MQTT (público como HiveMQ o privado con Mosquitto)

---

## 📐 Esquema de Conexiones

📌 **[Inserta aquí una imagen del diagrama de conexiones]**

Conexiones básicas:
- ZMPT101B → Pin analógico GPIO34 del ESP32
- Relé → GPIO25

---

## 🔧 Instalación del Entorno ESP32

1. **Instala Arduino IDE**:  
   Descárgalo desde [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

2. **Agrega soporte para placas ESP32**:
   - Ve a `Archivo > Preferencias`
   - Añade:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Luego, en `Herramientas > Placa > Gestor de tarjetas`, instala `esp32`

3. **Instala las librerías necesarias**:
   - `ZMPT101B` (desde el Library Manager)
   - `WiFi.h`
   - `PubSubClient`
   - `EEPROM`

---

## 📡 Configuración del ESP32

Este sistema utiliza:
- El pin `34` como entrada analógica del ZMPT101B
- El pin `25` para controlar el relé
- Conexión WiFi para enviar datos al broker MQTT
- EEPROM para almacenar eventos de sobretensión

### Configura tu red WiFi y MQTT en el código:

```cpp
const char* nombre_wifi = "TuSSID";
const char* contraseña_wifi = "TuPassword";

const char* direccion_mqtt = "broker.hivemq.com";
const int puerto_mqtt = 1883;
const char* nombre_dispositivo = "ESP32_Cortapicos";
const char* topico_general = "micro/6to/#";
```

### Umbral de protección:
Puedes ajustar el umbral de detección cambiando esta línea:
```cpp
const float VOLTAJE_LIMITE = 90.0f;
```

---

## ⚙️ Lógica del Sistema

1. El sensor mide el voltaje en tiempo real (RMS)
2. Si se detecta un voltaje mayor o igual al umbral:
   - Se activa el relé (corta el paso de corriente)
   - Se publica el evento por MQTT
   - Se almacena el evento en EEPROM (evita duplicados)
3. Si el voltaje vuelve a niveles normales:
   - Se desactiva el relé
   - Se reinicia la bandera de evento

---

## 🌐 MQTT y Monitoreo Remoto

El sistema publica los siguientes tópicos:
- `micro/6to/voltaje`: valor del voltaje medido (cada 500ms)
- `micro/6to/sobretension`: `ON` o `OFF` dependiendo del estado

También puede recibir comandos en:
- `micro/6to/apagado`: permite apagar o encender el relé manualmente (`APAGAR` / `ENCENDER`)

---

## 💾 Registro de Eventos en EEPROM

Cada evento de sobretensión es almacenado usando `EEPROM.writeFloat()`.

⚠️ El código verifica si la memoria está llena antes de guardar. Si se agota la EEPROM (512 bytes), los eventos nuevos no se guardarán.

---

## 🧪 Prueba del Sistema

1. Alimenta el ESP32 y asegúrate de que esté conectado al WiFi
2. Abre el monitor serial a 115200 baudios
3. Observa los valores de voltaje en tiempo real
4. Simula una sobretensión y verifica:
   - Activación del relé
   - Mensaje MQTT enviado
   - Registro del evento en EEPROM

---

## ☁️ ¿Cómo usar Mosquitto en local?

### En Linux:
```bash
sudo apt update
sudo apt install mosquitto mosquitto-clients
sudo systemctl start mosquitto
```

### En Windows:
Descarga e instala desde [https://mosquitto.org/download](https://mosquitto.org/download)

---

## 📁 Archivos del Repositorio

| Archivo                        | Descripción                                  |
|-------------------------------|----------------------------------------------|
| `detector_sobretension.ino`   | Código principal del ESP32                   |
| `diagrama_conexion.png`       | Esquema de conexiones (añadir imagen)        |

---

## 🤝 Contribuciones

¡Este es un proyecto de código abierto! Si deseas contribuir con mejoras o nuevas ideas, no dudes en abrir un *pull request* o *issue*.

---

## 🧑‍💻 Autor

Desarrollado por [**HenryGC**](https://github.com/Henry-GC)  
📍 Esmeraldas, Ecuador  
📬 Contacto: [contacto@henrygc.com]

---

## 📜 Licencia

Distribuido bajo la licencia **MIT**.

---

## 🌟 Agradecimientos

- Comunidad Arduino
- Proyecto ZMPT101B
- HiveMQ y Mosquitto
- Tutoriales y foros de electrónica aplicada

---

¡Con este sistema podrás proteger tus dispositivos contra sobrevoltajes peligrosos de forma económica y confiable!
