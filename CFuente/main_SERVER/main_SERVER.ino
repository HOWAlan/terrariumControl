#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "ControlActuador.h"
#include <Adafruit_NeoPixel.h>
#include "RGBLed.h"

#define PIN_R 21
#define PIN_G 20
#define PIN_B 19

#define PIN_PUMP 6 //CAMBIAR POR 3
#define PIN_HUM 0 
#define PIN_LIGHT 1

#define RETRIES_MAX 3
#define NUM_ACTUATORS 3

// Credenciales de tu Red WiFi
const char* ssid = "IZZI-343A";
const char* password = "RtPzaDTx";
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>


// Configuración de Hora (NTP) para México (Centro) - Ajusta tu zona horaria si es necesario
const long gmtOffset_sec = -21600; // -6 horas (CDMX)
const int daylightOffset_sec = 0;   // Sin horario de verano actual o ajustable

// Definición de pines y variables de control
const int OUTPUT_PIN = 2; // Pin GPIO para la salida (ej. LED integrado o relé)

int targetHour = 8;       // Hora programada por defecto (0-23)
int targetMinute = 0;     // Minuto programado por defecto (0-59)
int durationMinutes = 5;  // Duración en minutos por defecto

bool outputState = false;
unsigned long activationStartTime = 0;
unsigned long durationMillis = 0;

WebServer server(80);

// Código HTML del Dashboard Moderno
const char DASHBOARD_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32-C6 Terrarium Dashboard</title>
  <style>
    :root {
      --bg-color: #0f172a;
      --card-bg: rgba(30, 41, 59, 0.7);
      --accent: #38bdf8;
      --accent-hover: #0ea5e9;
      --text-main: #f8fafc;
      --text-muted: #94a3b8;
      --success: #22c55e;
      --danger: #ef4444;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Segoe UI', system-ui, sans-serif; }
    body {
      background-color: var(--bg-color);
      color: var(--text-main);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 20px;
    }
    header {
      margin-bottom: 30px;
      text-align: center;
    }
    h1 { font-size: 1.8rem; font-weight: 600; color: var(--accent); }
    p.subtitle { color: var(--text-muted); font-size: 0.9rem; margin-top: 5px; }
    
    .dashboard-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px;
      width: 100%;
      max-width: 900px;
    }
    .card {
      background: var(--card-bg);
      backdrop-filter: blur(12px);
      border: 1px solid rgba(255, 255, 255, 0.1);
      border-radius: 16px;
      padding: 24px;
      box-shadow: 0 10px 25px -5px rgba(0, 0, 0, 0.3);
    }
    .card h2 {
      font-size: 1.2rem;
      margin-bottom: 20px;
      display: flex;
      align-items: center;
      gap: 10px;
      border-bottom: 1px solid rgba(255, 255, 255, 0.1);
      padding-bottom: 10px;
    }
    .status-container {
      display: flex;
      flex-direction: column;
      gap: 15px;
    }
    .status-row {
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 1rem;
    }
    .badge {
      padding: 6px 12px;
      border-radius: 20px;
      font-weight: 600;
      font-size: 0.85rem;
      text-transform: uppercase;
    }
    .badge.off { background-color: rgba(239, 68, 68, 0.2); color: var(--danger); border: 1px solid var(--danger); }
    .badge.on { background-color: rgba(34, 197, 94, 0.2); color: var(--success); border: 1px solid var(--success); }

    .form-group {
      margin-bottom: 15px;
    }
    label {
      display: block;
      color: var(--text-muted);
      font-size: 0.85rem;
      margin-bottom: 6px;
    }
    input {
      width: 100%;
      padding: 10px 14px;
      background: rgba(15, 23, 42, 0.6);
      border: 1px solid rgba(255, 255, 255, 0.2);
      border-radius: 8px;
      color: white;
      font-size: 1rem;
      outline: none;
      transition: border-color 0.2s;
    }
    input:focus { border-color: var(--accent); }
    
    .btn {
      width: 100%;
      background-color: var(--accent);
      color: #0f172a;
      font-weight: 600;
      padding: 12px;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      font-size: 1rem;
      transition: background-color 0.2s, transform 0.1s;
      margin-top: 10px;
    }
    .btn:hover { background-color: var(--accent-hover); }
    .btn:active { transform: scale(0.98); }
    
    #toast {
      margin-top: 15px;
      text-align: center;
      font-size: 0.85rem;
      color: var(--success);
      min-height: 20px;
    }
  </style>
</head>
<body>

  <header>
    <h1>ESP32-C6 Control Panel</h1>
    <p class="subtitle">Sistema de Automatización y Salidas Temporizadas</p>
  </header>

  <div class="dashboard-grid">
    <!-- Tarjeta de Estado -->
    <div class="card">
      <h2>📊 Estado del Sistema</h2>
      <div class="status-container">
        <div class="status-row">
          <span>Salida Activa:</span>
          <span id="outStatus" class="badge off">Apagado</span>
        </div>
        <div class="status-row">
          <span>Hora Actual:</span>
          <strong id="currentTime">--:--:--</strong>
        </div>
        <div class="status-row">
          <span>Hora Programada:</span>
          <strong id="schedTime">--:--</strong>
        </div>
        <div class="status-row">
          <span>Duración Configurada:</span>
          <strong id="schedDur">-- min</strong>
        </div>
      </div>
    </div>

    <!-- Tarjeta de Configuración -->
    <div class="card">
      <h2>⚙️ Configuración de Salida</h2>
      <form id="configForm" onsubmit="saveConfig(event)">
        <div class="form-group">
          <label for="hour">Hora de Activación (0 - 23):</label>
          <input type="number" id="hour" min="0" max="23" required>
        </div>
        <div class="form-group">
          <label for="minute">Minuto de Activación (0 - 59):</label>
          <input type="number" id="minute" min="0" max="59" required>
        </div>
        <div class="form-group">
          <label for="duration">Duración (minutos):</label>
          <input type="number" id="duration" min="1" max="1440" required>
        </div>
        <button type="submit" class="btn">Guardar Cambios</button>
        <div id="toast"></div>
      </form>
    </div>
  </div>

  <script>
    async function fetchStatus() {
      try {
        const response = await fetch('/status');
        const data = await response.json();
        
        // Actualizar UI
        document.getElementById('currentTime').innerText = data.time;
        document.getElementById('schedTime').innerText = String(data.targetH).padStart(2, '0') + ':' + String(data.targetM).padStart(2, '0');
        document.getElementById('schedDur').innerText = data.duration + ' min';
        
        const badge = document.getElementById('outStatus');
        if (data.state) {
          badge.className = 'badge on';
          badge.innerText = 'Encendido';
        } else {
          badge.className = 'badge off';
          badge.innerText = 'Apagado';
        }
      } catch (err) {
        console.error('Error al obtener estado:', err);
      }
    }

    async function saveConfig(e) {
      e.preventDefault();
      const h = document.getElementById('hour').value;
      const m = document.getElementById('minute').value;
      const d = document.getElementById('duration').value;

      try {
        const response = await fetch(`/set?hour=${h}&minute=${m}&duration=${d}`);
        if (response.ok) {
          const toast = document.getElementById('toast');
          toast.innerText = '¡Configuración guardada con éxito!';
          setTimeout(() => toast.innerText = '', 3000);
          fetchStatus();
        }
      } catch (err) {
        alert('Error al guardar la configuración');
      }
    }

    // Cargar valores iniciales en los inputs al abrir la página
    async function init() {
      const response = await fetch('/status');
      const data = await response.json();
      document.getElementById('hour').value = data.targetH;
      document.getElementById('minute').value = data.targetM;
      document.getElementById('duration').value = data.duration;
      fetchStatus();
    }

    init();
    // Actualizar cada 3 segundos
    setInterval(fetchStatus, 3000);
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", DASHBOARD_page);
}

// Ruta para entregar el estado en formato JSON al navegador
void handleStatus() {
  struct tm timeinfo;
  char timeStr[9] = "--:--:--";
  
  if (getLocalTime(&timeinfo)) {
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  }

  String json = "{";
  json += "\"time\":\"" + String(timeStr) + "\",";
  json += "\"targetH\":" + String(targetHour) + ",";
  json += "\"targetM\":" + String(targetMinute) + ",";
  json += "\"duration\":" + String(durationMinutes) + ",";
  json += "\"state\":" + String(outputState ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

// Ruta para recibir los nuevos parámetros configurados por el usuario
void handleSet() {
  if (server.hasArg("hour")) targetHour = server.arg("hour").toInt();
  if (server.hasArg("minute")) targetMinute = server.arg("minute").toInt();
  if (server.hasArg("duration")) durationMinutes = server.arg("duration").toInt();

  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, LOW);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n¡Conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Configurar sincronización horaria NTP
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");

  // Configurar rutas del servidor web
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/set", handleSet);

  server.begin();
  Serial.println("Servidor web iniciado.");
}

void loop() {
  server.handleClient();

  // Lógica de tiempo para activar la salida
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    // Comprobar si coincide la hora y el minuto configurados
    if (timeinfo.tm_hour == targetHour && timeinfo.tm_min == targetMinute && timeinfo.tm_sec == 0) {
      if (!outputState) {
        outputState = true;
        digitalWrite(OUTPUT_PIN, HIGH);
        activationStartTime = millis();
        durationMillis = (unsigned long)durationMinutes * 60000UL; // Convertir minutos a milisegundos
        Serial.println("¡Salida activada por horario!");
      }
    }
  }

  // Comprobar si el tiempo de activación ha expirado
  if (outputState && (millis() - activationStartTime >= durationMillis)) {
    outputState = false;
    digitalWrite(OUTPUT_PIN, LOW);
    Serial.println("Salida desactivada (tiempo cumplido).");
  }
}