#include "unity.h"
#include "../src/veml3328.h"


/* Dummy i2c backend */
static uint8_t dummy_written_buf[8];
static int dummy_written_length;
static uint8_t dummy_read_buf[8];
static int dummy_fail = 0;

int i2c_write_bytes(int fd, uint8_t addr, uint8_t *buf, int length) {
    if (dummy_fail) {
        return -1;
    }

    for (int i = 0; i < length && i < 8; i++) {
        dummy_written_buf[i] = buf[i];
    }
    dummy_written_length = length;
    return 0;
}

int i2c_read_bytes(int fd, uint8_t dev_addr, uint8_t *buf, int length) {
    if (dummy_fail) {
        return -1;
    }

    for (int i = 0; i < length && i < 8; i++) {
        buf[i] = dummy_read_buf[i];
    }
    return 0;
}

/* Test Functions */
void test_write_reg (void) {
    int ret = veml3328_write_reg(0, VEML3328_I2C_ADDR, 0x04, 0x1234);
    TEST_ASSERT_EQUAL_INT(VEML3328_OK, ret);
    
    TEST_ASSERT_EQUAL_HEX8(0x04, dummy_written_buf[0]); // Register
    TEST_ASSERT_EQUAL_HEX8(0x34, dummy_written_buf[1]); // LSB
    TEST_ASSERT_EQUAL_HEX8(0x12, dummy_written_buf[2]); // MSB
}

void test_read_reg (void) {
    // Setup dummy read buffer
    dummy_read_buf[0] = 0xCD; // LSB
    dummy_read_buf[1] = 0xAB; // MSB

    uint16_t out;
    int ret = veml3328_read_reg(0, VEML3328_I2C_ADDR, VEML3328_REG_RED, &out);
    TEST_ASSERT_EQUAL_INT(VEML3328_OK, ret);
    
    TEST_ASSERT_EQUAL_HEX16(0xABCD, out);
}

void test_read_raw (void) {
    // Setup dummy read buffer for RED, GREEN, BLUE
    // RED = 100, GREEN = 200, BLUE = 50
    uint16_t dummy_values[] = {100, 200, 50};

    dummy_read_buf[0] = dummy_values[0] & 0xFF;         // RED LSB
    dummy_read_buf[1] = (dummy_values[0] >> 8) & 0xFF;  // RED MSB  
    dummy_read_buf[2] = dummy_values[1] & 0xFF;         // GREEN LSB
    dummy_read_buf[3] = (dummy_values[1] >> 8) & 0xFF;  // GREEN MSB
    dummy_read_buf[4] = dummy_values[2] & 0xFF;         // BLUE LSB
    dummy_read_buf[5] = (dummy_values[2] >> 8) & 0xFF;  // BLUE MSB
    
    veml3328_raw_data_t data;
    int ret = veml3328_read_raw(0, VEML3328_I2C_ADDR, &data);
    TEST_ASSERT_EQUAL_INT(VEML3328_OK, ret);
    
    TEST_ASSERT_EQUAL_UINT16(dummy_values[0], data.red);
    TEST_ASSERT_EQUAL_UINT16(dummy_values[1], data.green);
    TEST_ASSERT_EQUAL_UINT16(dummy_values[2], data.blue);
}

void test_norm (void) {
    veml3328_raw_data_t raw = {100, 50, 25};
    veml3328_norm_rgb_t norm = veml3328_norm(&raw);

    TEST_ASSERT_FLOAT_WITHIN(0.01, 1.0, norm.red);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.5, norm.green);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.25, norm.blue);
}

void test_config (void) {
    int ret = veml3328_config(0, VEML3328_I2C_ADDR);
    TEST_ASSERT_EQUAL_INT(VEML3328_OK, ret);

    TEST_ASSERT_EQUAL_HEX8(VEML3328_REG_CONF, dummy_written_buf[0]); // Register
    TEST_ASSERT_EQUAL_HEX16(0x0000, (dummy_written_buf[1] | (dummy_written_buf[2] << 8))); // Config value
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_write_reg);
    RUN_TEST(test_read_reg);
    RUN_TEST(test_read_raw);
    RUN_TEST(test_norm);
    RUN_TEST(test_config);

    return UNITY_END();
}   