#include "ControlActuador.h"

ControlActuador::ControlActuador(int act_id, String nombre_act, uint8_t pin_num, TipoActuador actuator_type, LogicType _logic) {
    id = act_id;
    nombre = nombre_act;
    pin = pin_num;
    type = actuator_type;
    logic = _logic;
    ticks = 0;
    delay = 0;
    
    estado_actual = false;
    programado = false;
    hora_programada = "08:00";
    duracion_horas = 1;
    tiempo_encendido = 0;
    servido = false;

    pinMode(pin, OUTPUT);
    digitalWrite(pin, !logic);
}

void ControlActuador::actualizarHardware(bool estado) {
    estado_actual = estado;
    digitalWrite(pin, !(estado^logic));
}

void ControlActuador::toggle() {
    bool nuevo_estado = !estado_actual;
    time_t ahora;
    time(&ahora);

    if (nuevo_estado) {
        tiempo_encendido = ahora;
    } else {
        tiempo_encendido = 0;
    }

    actualizarHardware(nuevo_estado);
}

void ControlActuador::Shot(uint8_t ticks, uint delayTime)
{
    for(int i = 0; i<ticks; i++)
    {
            digitalWrite(pin,logic);
            delay(delayTime);
            digitalWrite(pin,!logic);
            delay(delayTime);
    }
    digitalWrite(pinOut,!logic);
}

void ControlActuador::programar(String hora_str, int duracion_h, bool prog) {
    hora_programada = hora_str;
    duracion_horas = duracion_h;
    programado = prog;
    Serial.println("Actuador programado");
}

void programar(String hora_str, uint8_t ticks_p, uint delay_p)
{
    hora_programada = hora_str;
    ticks = ticks_p;
    delay = delay_p;
    servido = false; 
}

long ControlActuador::obtenerTiempoRestanteSeg() {
    if (estado_actual && tiempo_encendido > 0) {
        time_t ahora;
        time(&ahora);
        
        long duracion_seg = duracion_horas * 3600;
        long transcurrido = (long)(ahora - tiempo_encendido);
        long restante = duracion_seg - transcurrido;
        
        return (restante > 0) ? restante : 0;
    }
    return 0;
}

bool ControlActuador::detActuatorState() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return false;
    }

    int hora_actual = timeinfo.tm_hour;
    int min_actual = timeinfo.tm_min;

    // Parse de "HH:MM"
    int sepIndex = hora_programada.indexOf(':');
    if (sepIndex == -1) return false;

    int horasOn = hora_programada.substring(0, sepIndex).toInt();
    int minutosOn = hora_programada.substring(sepIndex + 1).toInt();

    int currentMinutes = hora_actual * 60 + min_actual;
    int startMinutes = horasOn * 60 + minutosOn;
    int durationMinutes = duracion_horas * 60;
    int finalMinutes = (startMinutes + durationMinutes) % (24 * 60);

    if (startMinutes <= finalMinutes) {
        return (currentMinutes >= startMinutes && currentMinutes < finalMinutes);
    } else { // Cruza la medianoche
        return (currentMinutes >= startMinutes || currentMinutes < finalMinutes);
    }
}

void ControlActuador::verificarTemporizador() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return;
    }

    time_t ahora_timestamp;
    time(&ahora_timestamp);

    // -------------------------------------------------------------
    // 1. EVALUAR ENCENDIDO
    // -------------------------------------------------------------
     bool state = detActuatorState();

        if(type==ON_OFF_TYPE)
        {
            actualizarHardware(state);
        }
        else
        {
            if(state && !servido)
            {
                Shot(ticks,delay);
                servido = true;
            }
        }

}

void ControlActuador::toJsonObject(JsonObject& obj) {
    obj["id"] = id;
    obj["nombre"] = nombre;
    obj["estado_actual"] = estado_actual;
    obj["programado"] = programado;
    obj["hora_programada"] = hora_programada;
    obj["duracion_horas"] = duracion_horas;
    obj["tiempo_restante_seg"] = obtenerTiempoRestanteSeg();
}