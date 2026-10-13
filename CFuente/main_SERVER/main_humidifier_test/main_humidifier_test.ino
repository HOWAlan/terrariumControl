#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "ControlActuador.h"
#include <Adafruit_NeoPixel.h>
#include "Humidifier.h"
#define PIN 8

// --- CONFIGURACIÓN DE RED WIFI ---
const char* WIFI_SSID = "IZZI-343A";
const char* WIFI_PASS = "RtPzaDTx";
const int retriesMax = 3;

// Ajuste de Zona Horaria en segundos (Ejemplo: UTC-6 para México = -6 * 3600)
const long OFFSET_ZONA_HORARIA = -6 * 3600;
const int DAYLIGHT_OFFSET = 0; // Ajuste de horario de verano (si aplica)

// Servidores NTP
const char* NTP_SERVER = "pool.ntp.org";

// --- INSTANCIAS GLOBALES ---
// Instancia del actuador 'bomba' en el Pin 6 (Pin 6 de tu tarjeta)
//ControlActuador pump(1, "bomba", 6, ON_OFF_TYPE, 0);
Humidifier hum(6,ACTIVE_HIGH);

// Control de intervalos no bloqueantes (en milisegundos)
unsigned long ultimoTiempoRTC = 0;
const unsigned long INTERVALO_RTC = 10000; // 10 segundos

unsigned long ultimoTiempoActuador = 0;
const unsigned long INTERVALO_ACTUADOR = 5000; // 5 segundos

//Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);

// -------------------------------------------------------------
// FUNCIONES DE RED Y RELOJ
// -------------------------------------------------------------

/**
 * Establece la conexión a la red WiFi con un timeout de 15 segundos
 */
bool conectar_wifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.println("Conectando a la red WiFi...");

    unsigned long inicio = millis();
    while (WiFi.status() != WL_CONNECTED) {
        // Si transcurren más de 15 segundos (15000 ms), aborta el intento
        if (millis() - inicio > 15000) {
            Serial.println("Error: No se pudo conectar al WiFi (Timeout).");
            WiFi.disconnect(true);
            return false;
        }
        delay(1000);
    }

    Serial.print("\nConexión establecida. IP: ");
    Serial.println(WiFi.localIP());
    return true;
}
/**
 * Configura la sincronización automática del RTC interno vía NTP
 */
void sincronizar_rtc_ntp() {
    Serial.println("Obteniendo hora desde el servidor NTP...");
    
    // Configura la zona horaria y los servidores NTP
    configTime(OFFSET_ZONA_HORARIA, DAYLIGHT_OFFSET, NTP_SERVER);

    struct tm timeinfo;
    // Intenta obtener la hora local validando que el RTC se haya actualizado
    if (getLocalTime(&timeinfo, 5000)) { // Timeout de 5s para sincronizar
        Serial.println("RTC interno sincronizado con éxito.");
    } else {
        Serial.println("Error al sincronizar con el servidor NTP.");
    }
}

// -------------------------------------------------------------
// TAREAS PERIÓDICAS (Equivalentes a las corrutinas asyncio)
// -------------------------------------------------------------

/**
 * Consulta e imprime la fecha y hora almacenadas en el RTC interno
 */
void bucle_consulta_rtc() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Error al obtener la hora del RTC");
        return;
    }

    Serial.println("--- Lectura RTC Interno ---");
    Serial.printf("Fecha: %02d/%02d/%04d\n", 
                  timeinfo.tm_mday, 
                  timeinfo.tm_mon + 1, 
                  timeinfo.tm_year + 1900);
                  
    Serial.printf("Hora:  %02d:%02d:%02d\n", 
                  timeinfo.tm_hour, 
                  timeinfo.tm_min, 
                  timeinfo.tm_sec);
}

/**
 * Demonio de verificación del estado del actuador
 */
void actuatorDaemon() {
    Serial.println("verificando actuador");
    hum.Shot(4, 200);
}

// -------------------------------------------------------------
// CONFIGURACIÓN Y BUCLE PRINCIPAL
// -------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(100);
}

void loop() {
    unsigned long tiempoActual = millis();

    // Tarea 2: Demonio de evaluación del actuador cada 5 segundos
    if (tiempoActual - ultimoTiempoActuador >= INTERVALO_ACTUADOR) {
        ultimoTiempoActuador = tiempoActual;
        actuatorDaemon();
    }
}