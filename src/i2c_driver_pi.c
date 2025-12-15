#include "i2c_driver_pi.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

int i2c_open_bus(const char *dev_path) {
    
    int fd = open(dev_path, O_RDWR);
    if (fd < 0) {                           // Error opening file
        perror("Failed to open I2C bus");
        return -1;
    }   
    
    return fd;
}

void i2c_close_bus(int fd) {
    if (fd >= 0) {              // Only close if valid fd
        close(fd);
    }
}

static int i2c_set_slave(int fd, uint8_t dev_addr) {
    if (ioctl(fd, I2C_SLAVE, dev_addr) < 0) {
        perror("Failed to set I2C slave address");
        return -1;
    }
    
    return 0;
}

int i2c_write_bytes(int fd, uint8_t dev_addr, const uint8_t *buf, int length) {
    if (fd < 0 || buf == NULL || length <= 0) {
        errno = EINVAL;
        return -1;
    }
    
    if (i2c_set_slave(fd, dev_addr) < 0) {
        return -1;
    }
    
    ssize_t bytes_written = write(fd, buf, (size_t)length);
    if (bytes_written != length) {
        if (bytes_written <0) {
            perror("Failed to write to I2C device");
        } else {
            fprintf(stderr, "Partial write to I2C device: %zd of %d bytes\n", bytes_written, length);
        }
        
        return -1;
    }
    
    return 0;
}

int i2c_read_bytes(int fd, uint8_t dev_addr, uint8_t *buf, int length) {
    if (fd < 0 || buf == NULL || length <= 0) {
        errno = EINVAL;
        return -1;
    }
    
    if (i2c_set_slave(fd, dev_addr) < 0) {
        return -1;
    }
    
    ssize_t bytes_read = read(fd, buf, (size_t)length);
    if (bytes_read != length) {
        if (bytes_read < 0) {
            perror("Failed to read from I2C device");
        } else {
            fprintf(stderr, "Partial read from I2C device: %zd of %d bytes\n", bytes_read, length);
        }
        
        return -1;
    }
    
    return 0;
}

int i2c_write_byte(int fd, uint8_t dev_addr, uint8_t data) {
    return i2c_write_bytes(fd, dev_addr, &data, 1);
}

int i2c_read_byte(int fd, uint8_t dev_addr, uint8_t *out) {
    if (out == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    
    return i2c_read_bytes(fd, dev_addr, out, 1);
}

int i2c_write_read(int fd, uint8_t dev_addr, const uint8_t *write_buf, int write_length, uint8_t *read_buf, int read_length) {
    if (fd < 0 || write_buf == NULL || write_length <= 0 || read_buf == NULL || read_length <= 0) {
        errno = EINVAL;
        return -1;
    }
    
    struct i2c_msg msgs[2];
    // Write message
    msgs[0].addr = dev_addr;
    msgs[0].flags = 0;                      // Write
    msgs[0].len = (uint16_t)write_length;
    msgs[0].buf = (uint8_t *)write_buf;
    
    // Read message
    msgs[1].addr = dev_addr;
    msgs[1].flags = I2C_M_RD;               // Read
    msgs[1].len = (uint16_t)read_length;
    msgs[1].buf = read_buf;
    
    struct i2c_rdwr_ioctl_data data = {
        .msgs = msgs,
        .nmsgs = 2
    };

    if (ioctl(fd, I2C_RDWR, &data) < 0) {
        perror("Failed to perform I2C write-read transaction");
        return -1;
    }
    
    return 0;
    
}