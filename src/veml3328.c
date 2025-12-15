#include "veml3328.h"
#include <stdint.h>
#include <stddef.h>

/* external backend functions */
extern int i2c_write_bytes(int fd, uint8_t dev_addr, const uint8_t *buf, int length);
//extern int i2c_read_bytes(int fd, uint8_t dev_addr, uint8_t *buf, int length);
extern int i2c_write_read(int fd, uint8_t dev_addr, const uint8_t *write_buf, int write_length, uint8_t *read_buf, int read_length);

/* Sensor initial configuration */
int veml3328_config(int i2c_fd, uint8_t dev_addr) {
    uint16_t conf_value = 0x0000; // Default config
    return veml3328_write_reg(i2c_fd, dev_addr, VEML3328_REG_CONF, conf_value);
}

/* write 16-bit register */
int veml3328_write_reg(int i2c_fd, uint8_t dev_addr, uint8_t reg, uint16_t value) {
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = value & 0xFF;         // LSB
    buf[2] = (value >> 8) & 0xFF;  // MSB

    if (i2c_write_bytes(i2c_fd, dev_addr, buf, 3) != 0){
        return VEML3328_ERR_I2C;
    }
    
    return VEML3328_OK;
}

/* read 16-bit register */
int veml3328_read_reg(int i2c_fd, uint8_t dev_addr, uint8_t reg, uint16_t *out) {
    if (out == NULL){
        return VEML3328_ERR_NULL;
    }
    
    /* Write register pointer
    if (i2c_write_bytes(i2c_fd, dev_addr, &reg, 1) != 0){
        return VEML3328_ERR_I2C;
    }*/

    uint8_t buf[2];
    // Read 2 bytes (LSB and MSB)
    if (i2c_write_read(i2c_fd, dev_addr, &reg, 1, buf, 2) != 0){
        return VEML3328_ERR_I2C;
    }

    *out = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8); // LSB | MSB
    return VEML3328_OK;
}

/* read raw color data */
int veml3328_read_all(int i2c_fd, uint8_t dev_addr, veml3328_raw_data_t *out) {
    if (out == NULL){
        return VEML3328_ERR_NULL;
    }

    if (veml3328_read_reg(i2c_fd, dev_addr, VEML3328_REG_clear, &out->clear) != VEML3328_OK ||
        veml3328_read_reg(i2c_fd, dev_addr, VEML3328_REG_RED, &out->red) != VEML3328_OK || 
        veml3328_read_reg(i2c_fd, dev_addr, VEML3328_REG_GREEN, &out->green) != VEML3328_OK || 
        veml3328_read_reg(i2c_fd, dev_addr, VEML3328_REG_BLUE, &out->blue) != VEML3328_OK){
        
        return VEML3328_ERR_I2C;
    }

    return VEML3328_OK;
}

/* Convert counts to irradiance (µW/cm^2) 
   R_eff = base * gain * (it_ms / ds_it_ms) * sense * dg; */
float veml3328_counts_to_irradiance(const veml3328_cfg_t *cfg, uint16_t counts) {
    if (cfg == NULL){
        return 0.0f; // Clamp to zero
    }

    int32_t corrected = (int32_t)counts - (int32_t)cfg->dark_offset;
    if (corrected < 0) {
        return 0.0f; // Clamp to zero
    }

    float R_eff = VEML3328_CLEAR_RESP_BASE * cfg->gain_factor * (cfg->it_ms / cfg->ds_it_ms) * cfg->sens_factor * cfg->dg_factor;
    if (R_eff <= 0.0f) {
        return 0.0f; // Clamp to zero
    }
    
    float irradiance = (float)(corrected) / R_eff;
    return irradiance;
}

float veml3328_estimate_wavelength(const veml3328_raw_data_t *raw) {
    if (raw == NULL){
        return 0.0f;
    }

    uint32_t sum = raw->red + raw->green + raw->blue;
    if (sum == 0) {
        return 0.0f;
    }
    float wavelength = (raw->red * VEML3328_WAVELENGTH_RED + raw->green * VEML3328_WAVELENGTH_GREEN +raw->blue * VEML3328_WAVELENGTH_BLUE) / (float)sum;
    
    return wavelength;
}

/* convert raw data to normalized RGB */
veml3328_norm_rgb_t veml3328_norm_colour(const veml3328_raw_data_t *raw, const veml3328_cfg_t *cfg) {
    veml3328_norm_rgb_t out;
    out.red = out.green = out.blue = 0.0f;
    out.intensity_counts = 0;
    out.irradiance_uW_per_cm2 = 0.0f;
    
    if (raw == NULL || cfg == NULL)
    {
        return out;
    }
    
    int32_t corrected = (int32_t)raw->clear - (int32_t)cfg->dark_offset;
    if(corrected < 0) {
        out.intensity_counts = 0;
    } else {
        out.intensity_counts = (uint16_t)corrected;
    }
    out.irradiance_uW_per_cm2 = veml3328_counts_to_irradiance(cfg, out.intensity_counts);

    uint32_t sum = raw->red + raw->green + raw->blue;
    if (sum == 0) {
        out.red = 0.0f;
        out.green = 0.0f;
        out.blue = 0.0f;
    } else {
        out.red = (float)raw->red / sum;
        out.green = (float)raw->green / sum;
        out.blue = (float)raw->blue / sum;
    }

    out.wavelength = veml3328_estimate_wavelength(raw);
    return out;
}