#include <stdio.h>
#include <stdlib.h>

#include "i2c_driver_pi.h"
#include "veml3328.h"
#include "tca9548a.h"

#define I2C_DEV_PATH    "/dev/i2c-1"  // verificar na Raspberry com o comando "ls /dev/i2c* "
#define TCA9548A_ADDR   0x70
#define VEML3328_ADDR   VEML3328_I2C_ADDR
#define NUM_CHANNELS    8

/* Config used for normalization */
static const veml3328_cfg_t default_cfg = {
    .gain_factor = 1.0f,
    .dg_factor   = 1.0f,
    .sens_factor = 1.0f,
    .it_ms       = 100.0f,
    .ds_it_ms    = 100.0f,
    .dark_offset = 0
};

int main(void) {
    /* Open I2C bus on the Pi */
    int fd = i2c_open_bus(I2C_DEV_PATH);
    if (fd < 0) {
        fprintf(stderr, "Failed to open %s bus\n", I2C_DEV_PATH);
        return 1;
    }

    printf("I2C bus opened on %s (fd = %d)\n", I2C_DEV_PATH, fd);

    /* Loop over all mux channels */
    for (int channel = 0; channel < NUM_CHANNELS; channel++) {
        
        /* Select channel TCA9548A */
        if (tca_select_channel(fd, TCA9548A_ADDR, channel) != TCA_OK) {
            fprintf(stderr, "Failed to select TCA9548A channel %d\n", channel);
            continue;
        }

        /* Configure VEML3328 on this channel */
        if (veml3328_config(fd, VEML3328_ADDR) != VEML3328_OK) {
            fprintf(stderr, "Failed to configure VEML3328 on channel %d\n", channel);
            continue;
        }

        /* Read raw data from VEML3328 */
        veml3328_raw_data_t raw_data;
        if (veml3328_read_all(fd, VEML3328_ADDR, &raw_data) != VEML3328_OK) {
            fprintf(stderr, "Failed to read data from VEML3328 on channel %d\n", channel);
            continue;
        }

        /* Compute relative RGB */
        veml3328_norm_rgb_t norm = veml3328_norm_colour(&raw_data, &default_cfg);

        printf("Channel %d - R: %.3f, G: %.3f, B: %.3f, Intensity: %u counts, Irradiance: %.3f µW/cm², Wavelength: %.1f nm\n",
               channel,
               norm.red,
               norm.green,
               norm.blue,
               norm.intensity_counts,
               norm.irradiance_uW_per_cm2,
               norm.wavelength);
    }

    /* Close bus */
    i2c_close_bus(fd);
    return EXIT_SUCCESS;
}