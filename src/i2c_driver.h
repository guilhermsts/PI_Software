#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <stdint.h>

/* Open an I2C bus. Returns fd or -1 on error. */
int i2c_open_bus(const char *dev_path);

/* Close a previously opened bus. Safe to call with fd < 0. */
void i2c_close_bus(int fd); 

/*
 * write "length" bytes from buf to device at 7-bit address 'dev_addr' on bus 'fd'.
 * Returns 0 on success, -1 on error.
 */
int i2c_write_bytes(int fd, uint8_t dev_addr, const uint8_t *buf, int length);

/*
 * read "length" bytes into buf from device at 7-bit address 'dev_addr' on bus 'fd'.
 * Returns 0 on success, -1 on error.
 */
int i2c_read_bytes(int fd, uint8_t dev_addr, uint8_t *buf, int length);

int i2c_write_byte(int i2c_fd, uint8_t dev_addr_7bit, uint8_t data);

int i2c_read_byte(int i2c_fd, uint8_t dev_addr_7bit, uint8_t *out);

#endif // I2C_DRIVER_H