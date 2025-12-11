#include "tca9548a.h"
#include <stdint.h>
#include <stddef.h>

extern int i2c_write_byte(int i2c_fd, uint8_t dev_addr_7bit, uint8_t data);
extern int i2c_read_byte(int i2c_fd, uint8_t dev_addr_7bit, uint8_t *out);

uint8_t tca_encode_channel(int channel) {
    if (channel < 0 || channel > 7) {
        return TCA_INVALID_CHANNEL;
    }
    return (1 << channel);
}

int tca_write_control(int i2c_fd, uint8_t dev_addr_7bit, uint8_t control) {
    int ret = i2c_write_byte(i2c_fd, dev_addr_7bit, control);
    if (ret != 0) {
        return TCA_ERR_I2C_WRITE;
    }
    return TCA_OK;
}

int tca_read_control(int i2c_fd, uint8_t dev_addr_7bit, uint8_t *out_control) {
    if (out_control == NULL)
    {
        return TCA_ERR_I2C_READ;
    }
    
    int ret = i2c_read_byte(i2c_fd, dev_addr_7bit, out_control);
    if (ret != 0) {
        return TCA_ERR_I2C_READ;
    }

    return TCA_OK;
}

int tca_select_channel(int i2c_fd, uint8_t dev_addr_7bit, int channel) {
    uint8_t control = tca_encode_channel(channel);
    if (control == TCA_INVALID_CHANNEL) {
        return TCA_INVALID_CHANNEL;
    }
    return tca_write_control(i2c_fd, dev_addr_7bit, control);
}

int tca_disable_all(int i2c_fd, uint8_t dev_addr_7bit) {
    return tca_write_control(i2c_fd, dev_addr_7bit, 0x00);
}