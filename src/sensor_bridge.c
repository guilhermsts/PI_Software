#include <stdint.h>

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

typedef struct {
    float R;
    float G;
    float B;
    float Intensity;
    float Wavelength;
} SensorData;

#ifndef _WIN32

#include <unistd.h>
#include "i2c_driver_pi.h"
#include "veml3328.h"
#include "tca9548a.h"

#define I2C_DEV_PATH "/dev/i2c-1"
#define TCA9548A_ADDR 0x70
#define VEML3328_ADDR VEML3328_I2C_ADDR

static const veml3328_cfg_t bridge_cfg_default = {
    .gain_factor = 4.0f,
    .dg_factor   = 2.0f,
    .sens_factor = 0.0f,
    .it_ms       = 400.0f,
    .ds_it_ms    = 100.0f,
    .dark_offset = 0
};

static float clamp01(float x) {
    if(x <= 0.0f) {
        return 0.0f;
    }
    if(x >= 1.0f) {
        return 1.0f;
    }

    return x;
}

static float rgb_255(float x) {
    x = clamp01(x);
    return (float)((int)(x * 255.0f + 0.5f));
}

EXPORT SensorData get_sensor_readings(int channel, int sensivity) {
    SensorData out = {0};

    if (channel < 0 || channel > 7) {
        return out;
    }

    int i2c_fd = i2c_open_bus(I2C_DEV_PATH);
    if(i2c_fd < 0) {
        return out;
    }

    (void)tca_disable_all(i2c_fd, TCA9548A_ADDR);
    if(tca_select_channel(i2c_fd, TCA9548A_ADDR, channel) != TCA_OK) {
        i2c_close_bus(i2c_fd);
        return out;
    }

    if (veml3328_config(i2c_fd, VEML3328_ADDR) != VEML3328_OK) {
        i2c_close_bus(i2c_fd);
        return out;
    }

    veml3328_cfg_t cfg = bridge_cfg_default;
    cfg.sens_factor = (sensivity != 0);

    (void)veml3328_apply_cfg(i2c_fd, VEML3328_ADDR, &cfg);
    
    usleep( (useconds_t)(cfg.it_ms * 1000.0f) ); // Wait for integration time

    veml3328_raw_data_t raw_data;
    if(veml3328_read_all(i2c_fd, VEML3328_ADDR, &raw_data) != VEML3328_OK) {
        i2c_close_bus(i2c_fd);
        return out;
    }

    veml3328_norm_rgb_t norm = veml3328_norm_colour(&raw_data, &cfg);
    out.R = rgb_255(norm.red);
    out.G = rgb_255(norm.green);
    out.B = rgb_255(norm.blue)  ;
    out.Intensity = norm.irradiance_uW_per_cm2;
    out.Wavelength = norm.wavelength;

    fprintf(stderr,
        "DBG bridge: raw C=%u R=%u G=%u B=%u | irr=%.3f wl=%.1f\n",
        raw_data.clear, raw_data.red, raw_data.green, raw_data.blue,
        norm.irradiance_uW_per_cm2, norm.wavelength);

    
    (void)tca_disable_all(i2c_fd, TCA9548A_ADDR);
    i2c_close_bus(i2c_fd);
    return out;
}

#else /* ---------------- Windows implementation Mock ---------------- */

EXPORT SensorData get_sensor_readings(int channel, int sensivity) {
    (void)sensivity;
    SensorData out = {0};

    float base = (float)(channel + 1);
    out.R = 0.10f * base;
    out.G = 0.05f * base;
    out.B = 0.02f * base;
    out.Intensity = 100.0f * base;
    out.Wavelength = 500.0f + 5.0f* base;
    
    return out;
}

#endif