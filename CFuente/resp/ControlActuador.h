#ifndef CONTROL_ACTUADOR_H
#define CONTROL_ACTUADOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <time.h>

// Definición de tipos de actuador equivalente a Constants.py
enum TipoActuador {
    ON_OFF_TYPE = 0,
    SHOT_TYPE = 1
};

// Definición de tipos de actuador equivalente a Constants.py
enum LogicType {
    ACTIVE_LOW = LOW,
    ACTIVE_HIGH = HIGH
};
class ControlActuador {
private:
    int id;
    String nombre;
    uint8_t pin;
    TipoActuador type;
    LogicType logic;
    unsigned long on_time; // Tiempo de encendido en segundos (para BLINK_TYPE)
    
    // Estado interno
    bool estado_actual;
    bool programado;
    bool servido;
    String hora_programada; // Formato "HH:MM"
    int duracion_horas;     // Duración programada en horas
    int ticks;
    uint delay;
    
    // Control de tiempo interno (Epoch timestamp)
    time_t tiempo_encendido;

public:
    // Constructor con valores por defecto
    ControlActuador(int act_id, String nombre_act, uint8_t pin_num, TipoActuador actuator_type = ON_OFF_TYPE, LogicType _logic);

    void actualizarHardware(bool estado);
    void toggle();
    void Shot(uint8_t ticks, uint delayTime);
    void programar(String hora_str, int duracion_h, bool prog = true);
    void programar(String hora_str, uint8_t ticks, uint delay);
    long obtenerTiempoRestanteSeg();
    void verificarTemporizador();
    bool detActuatorState();
    
    // Genera la representación equivalente a to_dict() / JSON
    void toJsonObject(JsonObject& doc);
    
    // Getters auxiliares
    int getId() const { return id; }
    bool getEstadoActual() const { return estado_actual; }
};

#endif