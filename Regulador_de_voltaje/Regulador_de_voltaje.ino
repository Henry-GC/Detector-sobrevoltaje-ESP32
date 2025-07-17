#include <EEPROM.h>
#include <ZMPT101B.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Definición de pines
#define PIN_VOLTAJE 34    // Pin de entrada para el sensor ZMPT101B
#define PIN_RELEE 25      // Pin de salida para el relé

// Configuración WIFI Y MQTT
const char* nombre_wifi = "NETLIFE-esmsrcevallosb1";
const char* contraseña_wifi = "0801472630";

const char* direccion_mqtt = "broker.hivemq.com";
const int puerto_mqtt = 1883;
const char* nombre_dispositivo = "ESP32_Cortapicos";
const char* topico_general = "micro/6to/#";

WiFiClient esp32Client;
PubSubClient client(esp32Client);

// Configuración
const float SENSIBILIDAD_SENSOR = 185.0f;  // Sensibilidad del sensor ZMPT101B (ajustar según calibración)
const float FRECUENCIA_RED = 60.0f;        // Frecuencia de la red eléctrica (Hz)
const float VOLTAJE_LIMITE = 90.0f;        // Voltaje límite en voltios
const int DIRECCION_EEPROM = 0;            // Dirección inicial en la EEPROM
const int TAMANO_EEPROM = 512;             // Tamaño de EEPROM a inicializar

// Variables globales
bool eventoRegistrado = false;             // Bandera para evitar registros duplicados
int contadorEventos = 0;                   // Contador de eventos almacenados

// Objeto del sensor de voltaje
ZMPT101B voltageSensor(PIN_VOLTAJE, FRECUENCIA_RED);

void setup() {
  Serial.begin(115200);
  
  // Configuración de pines
  pinMode(PIN_RELEE, OUTPUT);             // Configura el pin del relé como salida
  digitalWrite(PIN_RELEE, LOW);           // Inicializa el relé apagado
  
  // Configuración del sensor
  voltageSensor.setSensitivity(SENSIBILIDAD_SENSOR);

  // Inicializar WIFI
  Serial.println("Iniciando conección wifi");
  WiFi.begin(nombre_wifi, contraseña_wifi);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  Serial.println("Conectado al wifi");

  // Inicializar MQTT
  Serial.println("Iniciando conección al broker MQTT");
  connectMqtt();
  client.setCallback(callback);
  
  // Inicializar EEPROM
  if (!EEPROM.begin(TAMANO_EEPROM)) {
    Serial.println("Error al inicializar EEPROM");
  }
  
  Serial.println("Protector de sobretensión iniciado");
  Serial.println("Voltaje límite configurado: " + String(VOLTAJE_LIMITE) + "V");
}

void connectMqtt() {
  client.setServer(direccion_mqtt, puerto_mqtt);
  while (!client.connect(nombre_dispositivo)) {
    delay(1000);
    Serial.println(".");
  }
  Serial.println("Conectado al broker MQTT");
  client.subscribe(topico_general);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Mensaje recibido en el topic ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  if (topic == "micro/6to/apagado") {
    if (message == "APAGAR") {
      digitalWrite(PIN_RELEE, HIGH);
    } else if ( message == "ENCENDER") {
      digitalWrite(PIN_RELEE, LOW);
    }
  }
}

void loop() {
  // Obtener el voltaje RMS directamente del sensor
  if (!client.connected()) {
    connectMqtt();
  }
  client.loop();

  float voltaje = voltageSensor.getRmsVoltage();
  
  // Mostrar el voltaje medido
  Serial.println("Voltaje medido: " + String(voltaje) + "V");
  client.publish("micro/6to/voltaje", String(voltaje).c_str());

  // Control del relé basado en el voltaje
  controlRele(voltaje);
  
  delay(500); // Pequeña pausa antes de la siguiente lectura
}

void controlRele(float voltaje) {
  if (voltaje >= VOLTAJE_LIMITE) {
    digitalWrite(PIN_RELEE, HIGH); // Activa el relé si el voltaje es alto
    Serial.println("¡SOBRETENSIÓN DETECTADA! - Relé activado");
    client.publish("micro/6to/sobretension", "ON");
    
    // Verifica si el evento ya fue registrado
    if (!eventoRegistrado) {
      guardarEventoEnEEPROM(voltaje); // Guarda el evento en la EEPROM
      eventoRegistrado = true;        // Marca el evento como registrado
    }
  } else {
    digitalWrite(PIN_RELEE, LOW); // Desactiva el relé si el voltaje es normal
    eventoRegistrado = false;     // Reinicia la bandera cuando el voltaje vuelve a la normalidad
    Serial.println("Voltaje normal - Relé desactivado");
    client.publish("micro/6to/sobretension", "OFF");
  }
}

// Función para guardar eventos en la EEPROM
void guardarEventoEnEEPROM(float voltaje) {
  int direccion = DIRECCION_EEPROM + contadorEventos * sizeof(float);
  
  // Verifica si hay espacio disponible en la EEPROM
  if (direccion + sizeof(float) > TAMANO_EEPROM) {
    Serial.println("EEPROM llena. No se pueden almacenar más eventos.");
    return;
  }
  
  // Guarda el voltaje en la EEPROM
  EEPROM.writeFloat(direccion, voltaje);
  EEPROM.commit();
  
  contadorEventos++;
  Serial.println("Evento de sobretensión guardado: " + String(voltaje) + "V");
}