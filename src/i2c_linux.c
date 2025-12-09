/* i2c_linux.c - minimal Linux /dev/i2c-X backend */
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>

/* i2c_fd in our API will be the Linux file descriptor returned by open("/dev/i2c-X", O_RDWR) */

int i2c_write_byte(int i2c_fd, uint8_t dev_addr_7bit, uint8_t data) {
    if (ioctl(i2c_fd, I2C_SLAVE, dev_addr_7bit) < 0) {
        return -1;
    }

    uint8_t buf = data;
    ssize_t w = write(i2c_fd, &buf, 1);
    
    return (w == 1) ? 0 : -1;
}

int i2c_read_byte(int i2c_fd, uint8_t dev_addr_7bit, uint8_t *out) {
    if (out == NULL) {
        return -1;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, dev_addr_7bit) < 0) {
        return -1;
    }

    uint8_t buf;
    ssize_t r = read(i2c_fd, &buf, 1);
    
    if (r != 1) {
        return -1;
    }

    *out = buf;
    return 0;
}
