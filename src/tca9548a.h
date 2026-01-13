#ifndef TCA9548A_H
#define TCA9548A_H

#include <stdint.h>

// Error codes
#define TCA_OK              0
#define TCA_ERR_I2C_WRITE   -1
#define TCA_ERR_I2C_READ    -2
#define TCA_INVALID_CHANNEL 0xFF

/* Considering A0, A1, A2 pins are grounded */
#define TCA_ADDRESS_BASE 0x70 

/* Conversion of a channel number into a control byte*/
uint8_t tca_encode_channel(int channel);

/* Writing the control byte to the multiplexer*/
int tca_write_control(int i2c_fd, uint8_t dev_addr_7bit, uint8_t control);

/* Reads back control register from TCA*/
int tca_read_control(int i2c_fd, uint8_t dev_addr_7bit, uint8_t *out_control);

/* Select a specific channel*/
int tca_select_channel(int i2c_fd, uint8_t dev_addr_7bit, int channel);

/* Disable all channels*/
int tca_disable_all(int i2c_fd, uint8_t dev_addr_7bit);






#endif // TCA9548A_H