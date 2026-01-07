#include "veml3328.h"
#include <stdint.h>
#include <stddef.h>

#define VEML3328_CONF_DG    12  // bits 13:12
#define VEML3328_CONF_GAIN  10  // bits 11:10
#define VEML3328_CONF_SENS  6   // bit  6
#define VEML3328_CONF_IT    4   // bits 5:4
/* Keeping SD1=0, SD0=0, SD_ALS=0, AF=0, TRIG=0, reserved=0 (default values) */

#define VEML3328_WAVELENGTH_RED     620.0f  // nm
#define VEML3328_WAVELENGTH_GREEN   550.0f  // nm
#define VEML3328_WAVELENGTH_BLUE    470.0f  // nm

/* external backend functions */
extern int i2c_write_bytes(int fd, uint8_t dev_addr, const uint8_t *buf, int length);
//extern int i2c_read_bytes(int fd, uint8_t dev_addr, uint8_t *buf, int length);
extern int i2c_write_read(int fd, uint8_t dev_addr, const uint8_t *write_buf, int write_length, uint8_t *read_buf, int read_length);

static uint16_t encode_it_bits(float it_ms) {
    // Datasheet: 00=50ms, 01=100ms, 10=200ms, 11=400ms
    if (it_ms <= 50.0f) {
        return 0b00;                // 50ms
    } else if (it_ms <= 100.0f) {
        return 0b01;                // 100ms
    } else if (it_ms <= 200.0f) {
        return 0b10;                // 200ms
    } else {        
        return 0b11;                // 400ms
    }
}

static uint16_t encode_gain_bits(float gain) {
    // Datasheet: 00=1x, 01=2x, 10=4x, 11=1/2x
    if (gain <= 0.75f) {
        return 0b11;                // 0.5x
    } else if (gain <= 1.5f) {
        return 0b00;                // 1x
    } else if (gain <= 3.0f) {
        return 0b01;                // 2x
    } else {
        return 0b10;                // 4x
    }
}

static uint16_t encode_dg_bits(float dg) {
    // Datasheet: 00=1x, 01=2x, 10=4x, 11=reserved (do not use)
    if (dg <= 1.5f) {
        return 0b00;                // 1x
    } else if (dg <= 3.0f) {
        return 0b01;                // 2x
    } else {
        return 0b10;                // 4x
    } 
}

static uint16_t encode_sens_bits(float sens) {
    // Datasheet: 0=normal, 1=high
    if (sens <= 0.7f) {
        return 1u;                  // high
    } else {
        return 0u;                  // low
    }
}

static float decode_it_ms(uint16_t conf) {
    uint16_t it_bits = (conf >> VEML3328_CONF_IT) & 0x3;
    switch (it_bits) {
        case 0b00: return 50.0f;
        case 0b01: return 100.0f;
        case 0b10: return 200.0f;
        case 0b11: return 400.0f;
    }
    return 50.0f; // default fallback
}

static float decode_gain(uint16_t conf) {
    uint16_t gain_bits = (conf >> VEML3328_CONF_GAIN) & 0x3;
    switch (gain_bits) {
        case 0b00: return 1.0f;
        case 0b01: return 2.0f;
        case 0b10: return 4.0f;
        case 0b11: return 0.5f;
    }
    return 1.0f; // default fallback
}

static float decode_dg(uint16_t conf) {
    uint16_t dg_bits = (conf >> VEML3328_CONF_DG) & 0x3;
    switch (dg_bits) {
        case 0b00: return 1.0f;
        case 0b01: return 2.0f;
        case 0b10: return 4.0f;
        default:   return 1.0f; // reserved, fallback to 1x
    }
}

static float decode_sens(uint16_t conf) {
    uint16_t sens_bit = (conf >> VEML3328_CONF_SENS) & 0x1;
    if (sens_bit == 0) {
        return 1.0f; // normal
    } else {
        return (1.0f/3.0f); // high
    }
}

/* Sensor initial configuration */
int veml3328_config(int i2c_fd, uint8_t dev_addr) {
    uint16_t conf_value = 0x0000; // Default config
    return veml3328_write_reg(i2c_fd, dev_addr, VEML3328_REG_CONF, conf_value);
}

int veml3328_apply_cfg(int i2c_fd, uint8_t dev_addr, const veml3328_cfg_t *cfg) {
    if (cfg == NULL){
        return VEML3328_ERR_NULL;
    }

    uint16_t conf_value = 0x0000;   // Start with default config

    conf_value |= (encode_dg_bits(cfg->dg_factor)       << VEML3328_CONF_DG);
    conf_value |= (encode_gain_bits(cfg->gain_factor)   << VEML3328_CONF_GAIN);
    conf_value |= (encode_sens_bits(cfg->sens_factor)   << VEML3328_CONF_SENS);
    conf_value |= (encode_it_bits(cfg->it_ms)           << VEML3328_CONF_IT);

    return veml3328_write_reg(i2c_fd, dev_addr, VEML3328_REG_CONF, conf_value);
}

int veml3328_read_cfg (int i2c_fd, uint8_t dev_addr, veml3328_cfg_t *cfg_out) {
    if (cfg_out == NULL){
        return VEML3328_ERR_NULL;
    }

    uint16_t conf_value = 0;
    int ret = veml3328_read_reg(i2c_fd, dev_addr, VEML3328_REG_CONF, &conf_value);
    if (ret != VEML3328_OK){
        return ret;
    }

    cfg_out->it_ms       = decode_it_ms(conf_value);
    cfg_out->gain_factor = decode_gain(conf_value);
    cfg_out->dg_factor   = decode_dg(conf_value);
    cfg_out->sens_factor = decode_sens(conf_value);
    
    // Note: ds_it_ms and dark_offset are not stored in config register
    cfg_out->ds_it_ms    = cfg_out->it_ms; // default to same as it_ms
    cfg_out->dark_offset = 0;               // default to 0

    return VEML3328_OK;
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

float veml3328_estimate_wavelength(uint16_t red, uint16_t green, uint16_t blue) {
    uint32_t sum = (uint32_t)red + (uint32_t)green + (uint32_t)blue;
    if (sum == 0){
        return 0.0f;
    }

    float wavelength = ((float)red * VEML3328_WAVELENGTH_RED + (float)green * VEML3328_WAVELENGTH_GREEN + (float)blue * VEML3328_WAVELENGTH_BLUE) / (float)sum;
    
    return wavelength;
}

/* convert raw data to normalized RGB */
veml3328_norm_rgb_t veml3328_norm_colour(const veml3328_raw_data_t *raw, const veml3328_cfg_t *cfg) {
    veml3328_norm_rgb_t out;
    out.red = out.green = out.blue = 0.0f;
    out.intensity_counts = 0;
    out.irradiance_uW_per_cm2 = 0.0f;
    out.wavelength = 0.0f;
    
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

    int32_t r_corr = (int32_t)raw->red - (int32_t)cfg->dark_offset;
    int32_t g_corr = (int32_t)raw->green - (int32_t)cfg->dark_offset;
    int32_t b_corr = (int32_t)raw->blue - (int32_t)cfg->dark_offset;

    if (r_corr < 0) r_corr = 0;
    if (g_corr < 0) g_corr = 0;
    if (b_corr < 0) b_corr = 0;

    uint32_t sum = (uint32_t)r_corr + (uint32_t)g_corr + (uint32_t)b_corr;
    if (sum == 0) {
        out.red = 0.0f;
        out.green = 0.0f;
        out.blue = 0.0f;
        out.wavelength = 0.0f;
    } else {
        out.red = (float)r_corr / sum;
        out.green = (float)g_corr / sum;
        out.blue = (float)b_corr / sum;

        out.wavelength = veml3328_estimate_wavelength((uint16_t)r_corr, (uint16_t)g_corr, (uint16_t)b_corr);
    }

    return out;
}