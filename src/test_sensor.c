#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "i2c_driver_pi.h"
#include "tca9548a.h"
#include "veml3328.h"

#define I2C_DEV_PATH    "/dev/i2c-1"
#define TCA9548A_ADDR   0x70
#define VEML3328_ADDR   VEML3328_I2C_ADDR

static const veml3328_cfg_t test_cfg = {
    .gain_factor = 1.0f,
    .dg_factor   = 1.0f,
    .sens_factor = 1.0f,
    .it_ms       = 100.0f,
    .ds_it_ms    = 100.0f,
    .dark_offset = 0
};

int main(int argc, char *argv[]) {
    if (argc < 2)
    {
        fprintf(stderr, "USAGE: %s <channel_index 0-7> [num_samples]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int channel = atoi(argv[1]);
    int num_samples = argc >= 3 ? atoi(argv[2]) : 1;
    
    if(channel < 0 || channel > 7) {
        fprintf(stderr, "ERROR: Channel index must be between 0 and 7.\n");
        return EXIT_FAILURE;
    }

    if (num_samples <= 0)
    {
        num_samples = 1;
    }
    
    printf("Testing TCA channel %d for %d samples(s).\n", channel, num_samples);

    /* Open I2C bus */
    int fd = i2c_open_bus(I2C_DEV_PATH);
    if (fd < 0) {
        fprintf(stderr, "ERROR: Unable to open I2C bus %s\n", I2C_DEV_PATH);
        return EXIT_FAILURE;
    }

    printf("I2C bus opened on %s (fd = %d)\n", I2C_DEV_PATH, fd);

    int ret = tca_disable_all(fd, TCA9548A_ADDR);
    if (ret != TCA_OK) {
        fprintf(stderr, "WARNING: tca_disable_all failed (ret=%d)\n", ret);
    }

    /* Select requested channel */
    if (tca_select_channel(fd, TCA9548A_ADDR, channel) != TCA_OK) {
        fprintf(stderr, "ERROR: Unable to select TCA9548A channel %d\n", channel);
        i2c_close_bus(fd);
        return EXIT_FAILURE;
    }

    printf("TCA9548A channel %d selected.\n", channel);

    /* Configure VEML3328 sensor */
    if (veml3328_config(fd, VEML3328_ADDR) != VEML3328_OK) {
        fprintf(stderr, "ERROR: Unable to configure VEML3328 sensor\n");
        i2c_close_bus(fd);
        return EXIT_FAILURE;
    }

    usleep(200000); // 200 ms
    for (int sample = 0; sample < num_samples; sample++) {
        /* Read raw data from VEML3328 */
        veml3328_raw_data_t raw_data;
        if (veml3328_read_all(fd, VEML3328_ADDR, &raw_data) != VEML3328_OK) {
            fprintf(stderr, "ERROR: Unable to read data from VEML3328 sensor\n");
            break;
        }

        printf("Raw: C=%u R=%u G=%u B=%u\n", raw_data.clear, raw_data.red, raw_data.green, raw_data.blue);

        /* Compute normalized RGB values */
        veml3328_norm_rgb_t norm_rgb = veml3328_norm_colour(&raw_data, &test_cfg);

        printf("Normalized RGB Values of sample %d:\n", sample + 1);
        printf("Red: %.3f  ,  Green: %.3f  ,  Blue:  %.3f\n", norm_rgb.red, norm_rgb.green, norm_rgb.blue);
        printf("Intensity: %.3f uW/cm^2  ,  Wavelength: %.1f\n", norm_rgb.irradiance_uW_per_cm2, norm_rgb.wavelength);

        usleep(1000000);  // 1s delay between samples
    }

    tca_disable_all(fd, TCA9548A_ADDR);

    /* Close I2C bus */
    i2c_close_bus(fd);
    return EXIT_SUCCESS;
}