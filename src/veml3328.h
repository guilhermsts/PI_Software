#ifndef VEMLL3328_H
#define VEMLL3328_H

#include <stdint.h>

/* Default I2C address */
#define VEML3328_I2C_ADDR 0x10

/* Error codes */
#define VEML3328_OK          0
#define VEML3328_ERR_I2C    -1 
#define VEML3328_ERR_NULL   -2

/* Register map */
#define VEML3328_REG_CONF   0x00
#define VEML3328_REG_clear  0x04
#define VEML3328_REG_RED    0x05
#define VEML3328_REG_GREEN  0x06
#define VEML3328_REG_BLUE   0x07

/* Clear channel base responsivity */
#define VEML3328_CLEAR_RESP_BASE 57.0f // from datasheet

/* Peak wavelengths from datasheet */
#define VEML3328_WAVELENGTH_RED    610.0f
#define VEML3328_WAVELENGTH_GREEN  560.0f
#define VEML3328_WAVELENGTH_BLUE   470.0f

/* Raw sensor data struct*/
typedef struct {
    uint16_t clear;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
} veml3328_raw_data_t;

/* Normalized RGB values */
typedef struct {
    float red;
    float green;
    float blue;
    uint16_t intensity_counts;
    float irradiance_uW_per_cm2;
    float wavelength;
} veml3328_norm_rgb_t;

typedef struct {
    float gain_factor;
    float dg_factor;
    float sens_factor;
    float it_ms;
    float ds_it_ms;
    uint16_t dark_offset;
} veml3328_cfg_t;

/* Sensor initial configuration */
int veml3328_config(int i2c_fd, uint8_t dev_addr);

/* write 16-bit address */
int veml3328_write_reg(int i2c_fd, uint8_t dev_addr, uint8_t reg,uint16_t value);

/* read 16-bit address */
int veml3328_read_reg(int i2c_fd, uint8_t dev_addr, uint8_t reg, uint16_t *out);

/* read raw color data */
int veml3328_read_all(int i2c_fd, uint8_t dev_addr, veml3328_raw_data_t *out);

/* Convert counts to irradiance */
float veml3328_counts_to_irradiance(const veml3328_cfg_t *cfg, uint16_t counts);

/* Wavelength estimation */
float veml3328_estimate_wavelength(const veml3328_raw_data_t *raw);

/* convert raw data to normalized RGB */
veml3328_norm_rgb_t veml3328_norm_colour(const veml3328_raw_data_t *raw, const veml3328_cfg_t *cfg);

#endif /* VEMLL3328_H */