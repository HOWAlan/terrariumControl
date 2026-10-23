#ifndef CONTROL_ACTUADOR_H
#define CONTROL_ACTUADOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <time.h>
#include "types.h"

enum TipoActuador {
    ON_OFF_TYPE = 0,
    SHOT_TYPE = 1
};

class ControlActuador {
private:
    int id;
    String nombre;
    uint8_t pin;
    TipoActuador type;
    LogicType logic;
    
    // Estado interno
    bool estado_actual;
    bool programado;
    bool servido;
    String hora_programada; // Formato "HH:MM"
    int duracion_horas;     // Duración programada en horas
    uint8_t ticks;
    uint32_t delay_ms;      // Renombrado para evitar conflicto con la función delay()
    
    // Control de tiempo interno
    time_t tiempo_encendido;

public:
    uint32_t tiempo_cambio;
    ControlActuador(int act_id, String nombre_act, uint8_t pin_num, TipoActuador actuator_type = ON_OFF_TYPE, LogicType _logic = ACTIVE_HIGH);

    void actualizarHardware(bool estado);
    void toggle();
    void Shot(uint8_t ticks, uint32_t delayTime);
    void programar(String hora_str, int duracion_h);
    void programar(String hora_str, uint8_t ticks_p, uint32_t delay_p, uint8_t duracion_h);
    long obtenerTiempoRestanteSeg();
    String CalculateNewOnHour(String oldHour, uint8_t period);
    void verificarTemporizador();
    bool detActuatorState();
    uint32_t ControlActuador::GetTimeTillChange();

    void toJsonObject(JsonObject& doc);
    
    int getId() const { return id; }
    bool getEstadoActual() const { return estado_actual; }
    String getActuatorInfo();
};

#endif