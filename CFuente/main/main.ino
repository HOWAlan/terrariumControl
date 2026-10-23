#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "ControlActuador.h"
#include "RGBLed.h"

#define PIN_R 20
#define PIN_G 19
#define PIN_B 15

#define PIN_PUMP 0 
#define PIN_HUM 1 
#define PIN_LIGHT 3

#define RETRIES_MAX 3

// --- CONFIGURACIÓN DE RED WIFI ---
const char* WIFI_SSID = "IZZI-343A";
const char* WIFI_PASS = "RtPzaDTx";

// Ajuste de Zona Horaria en segundos (UTC-6 para México)
const long OFFSET_ZONA_HORARIA = -6 * 3600;
const int DAYLIGHT_OFFSET = 0;

// Servidores NTP
const char* NTP_SERVER = "pool.ntp.org";

// --- INSTANCIAS GLOBALES ---
ControlActuador pump(1, "bomba", PIN_PUMP, ON_OFF_TYPE);
ControlActuador light(2, "lampara", PIN_LIGHT, ON_OFF_TYPE);
ControlActuador hum(3, "Humidificador", PIN_HUM, SHOT_TYPE);

// Arreglo global de actuadores
ControlActuador misActuadores[3] = {pump, light, hum};
const int NUM_ACTUADORES = 3;

RGBLed indicatorLed(PIN_R, PIN_G, PIN_B, ACTIVE_HIGH);

// Control de intervalos no bloqueantes (en milisegundos)
unsigned long ultimoTiempoRTC = 0;
const unsigned long INTERVALO_RTC = 30000; // 30 segundos

unsigned long ultimoTiempoActuador = 0;
const unsigned long INTERVALO_ACTUADOR = 30000; // 30 segundos

// -------------------------------------------------------------
// FUNCIONES DE RED Y RELOJ
// -------------------------------------------------------------

bool conectar_wifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.println("Conectando a la red WiFi...");

    unsigned long inicio = millis();
    while (WiFi.status() != WL_CONNECTED) {
        indicatorLed.ShowStatus(PROCESS_STATUS);

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

bool sincronizar_rtc_ntp() {
    Serial.println("Obteniendo hora desde el servidor NTP...");
    indicatorLed.ShowStatus(RTC_PROCESS_STATUS);
    configTime(OFFSET_ZONA_HORARIA, DAYLIGHT_OFFSET, NTP_SERVER);

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 5000)) {
        Serial.println("RTC interno sincronizado con éxito.");
        return true;
    } else {
        Serial.println("Error al sincronizar con el servidor NTP.");
        return false;
    }
}

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

// -------------------------------------------------------------
// TAREAS PERIÓDICAS
// -------------------------------------------------------------

// Se recibe el arreglo y su tamaño explícitamente para evitar errores de memoria
void actuatorDaemon(ControlActuador actuators[], int size) {
    Serial.println("______________________________________________");
    Serial.println("Verificando actuadores...");
    for (int i = 0; i < size; i++) {
        actuators[i].verificarTemporizador();
        Serial.println(actuators[i].getActuatorInfo());
        Serial.println("._._._._._._._._._._._._._._._._._._._._._._._._.");
    }
    Serial.println("______________________________________________");
}

// -------------------------------------------------------------
// CONFIGURACIÓN Y BUCLE PRINCIPAL
// -------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Configuración inicial de los actuadores
    pump.programar("08:00", 23);
    hum.programar("08:30", 4, 500,12);
    light.programar("08:00", 15);

    // Actualizamos el arreglo global con las instancias ya configuradas
    misActuadores[0] = pump;
    misActuadores[1] = light;
    misActuadores[2] = hum;

    unsigned int retries = 0;
    while (retries < RETRIES_MAX) {
        if (conectar_wifi()) {
            indicatorLed.ShowStatus(OK_STATUS);
            break;
        } else {
            indicatorLed.ShowStatus(ERROR_STATUS);
            delay(500);
            retries++;
            Serial.println("Reintentando Conexión...");
        }
    }
    retries = 0;
    delay(1000);
    while (retries < RETRIES_MAX) {
        if(sincronizar_rtc_ntp()){
            indicatorLed.ShowStatus(OK_STATUS);
            break;
        }else
        {
            indicatorLed.ShowStatus(ERROR_STATUS);
            delay(500);
            retries++;
            Serial.println("Reintentando obtener hora desde NTP");
        }
    }
    
    Serial.println("\nIniciando servicios en segundo plano...\n");
    
    // Llamada correcta pasando el arreglo y su tamaño
    actuatorDaemon(misActuadores, NUM_ACTUADORES);
}

void loop() {
    unsigned long tiempoActual = millis();

    if (tiempoActual - ultimoTiempoRTC >= INTERVALO_RTC) {
        ultimoTiempoRTC = tiempoActual;
        bucle_consulta_rtc();
        indicatorLed.ShowStatus(PROCESS_STATUS);
    }

    if (tiempoActual - ultimoTiempoActuador >= INTERVALO_ACTUADOR) {
        ultimoTiempoActuador = tiempoActual;
        actuatorDaemon(misActuadores, NUM_ACTUADORES);
        indicatorLed.ShowStatus(OK_STATUS);
    }
}