// sensors.c 
#include <stdio.h>
#include <stdbool.h> 


// Definimos a estrutura para facilitar o transporte de múltiplos dados
typedef struct {
    float R;
    float B;
    float G;
    float Intensity;
    float Wavelength;
} SensorResult;

// Função que será chamada pelo Python
SensorResult get_sensor_readings(int sensor_id, int sensitivity) {
    SensorResult data;

    // Aqui você colocaria a lógica real de leitura do hardware
    // Exemplo de preenchimento de dados:
    data.R = 10.5f * sensor_id;
    data.B = 20.2f;
    data.G = 30.1f;
    data.Intensity = 1.5f;
    data.Wavelength = 550.0f;

    return data;
}