#include "unity.h"
#include <string.h>
#include "../src/veml3328.h"


/* Dummy i2c backend */
static uint8_t current_reg = 0;
static uint16_t dummy_reg_values[256]; // Simulate 256 16-bit registers
static uint8_t dummy_written_buf[8];
static int dummy_written_length;
static uint8_t dummy_read_buf[8];
static int dummy_fail = 0;

int i2c_write_bytes(int fd, uint8_t addr, const uint8_t *buf, int length) {
    if (dummy_fail) {
        return -1;
    }

    if (length == 1)
    {
        current_reg = buf[0];
    } else {
        for (int i = 0; i < length && i < 8; i++) {
            dummy_written_buf[i] = buf[i];
        }
        dummy_written_length = length;
    }
    
    return 0;
}

int i2c_write_read(int fd, uint8_t dev_addr, const uint8_t *wbuf, int wlen, uint8_t *rbuf, int rlen) {
    (void)fd;
    (void)dev_addr;

    if (!wbuf || wlen != 1 || !rbuf || rlen != 2) {
        return -1;
    }
    uint8_t reg = wbuf[0];
    uint16_t val = dummy_reg_values[reg];

    // little-endian: LSB first, like your driver assembles it
    rbuf[0] = (uint8_t)(val & 0xFF);
    rbuf[1] = (uint8_t)(val >> 8);
    
    if (dummy_fail) {
        return -1;
    } else {
        return 0;
    }
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
    dummy_reg_values[VEML3328_REG_RED] = 0xABCD;

    uint16_t out;
    int ret = veml3328_read_reg(0, VEML3328_I2C_ADDR, VEML3328_REG_RED, &out);
    TEST_ASSERT_EQUAL_INT(VEML3328_OK, ret);
    
    TEST_ASSERT_EQUAL_HEX16(0xABCD, out);
}

void test_read_raw (void) {
    // Setup dummy read buffer for RED, GREEN, BLUE
    // RED = 100, GREEN = 200, BLUE = 50
    dummy_reg_values[VEML3328_REG_RED]   = 100;
    dummy_reg_values[VEML3328_REG_GREEN] = 200; 
    dummy_reg_values[VEML3328_REG_BLUE]  = 50;
    
    veml3328_raw_data_t data;
    int ret = veml3328_read_all(0, VEML3328_I2C_ADDR, &data);
    TEST_ASSERT_EQUAL_INT(VEML3328_OK, ret);
    
    TEST_ASSERT_EQUAL_UINT16(100, data.red);
    TEST_ASSERT_EQUAL_UINT16(200, data.green);
    TEST_ASSERT_EQUAL_UINT16(50, data.blue);
}

void test_norm (void) {
    veml3328_raw_data_t raw = {
        .clear = 100,
        .red = 50,
        .green = 25,
        .blue = 25
    };
    
    veml3328_cfg_t cfg = {
        .gain_factor = 1.0f,
        .dg_factor   = 1.0f,
        .sens_factor = 1.0f,
        .it_ms       = 100.0f,
        .ds_it_ms    = 100.0f,
        .dark_offset = 0
    };
    
    veml3328_norm_rgb_t norm = veml3328_norm_colour(&raw, &cfg);

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, norm.red);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.25f, norm.green);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.25f, norm.blue);
}

void test_config (void) {
    int ret = veml3328_config(0, VEML3328_I2C_ADDR);
    TEST_ASSERT_EQUAL_INT(VEML3328_OK, ret);

    TEST_ASSERT_EQUAL_HEX8(VEML3328_REG_CONF, dummy_written_buf[0]); // Register
    TEST_ASSERT_EQUAL_HEX16(0x0000, (dummy_written_buf[1] | (dummy_written_buf[2] << 8))); // Config value
}

void test_wavelength_red_pure(void) {
    float wl = veml3328_estimate_wavelength(255, 0, 0);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, VEML3328_WAVELENGTH_RED, wl);
}

void test_wavelength_green_pure(void) {
    float wl = veml3328_estimate_wavelength(0, 255, 0);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, VEML3328_WAVELENGTH_GREEN, wl);
}

void test_wavelength_blue_pure(void) {
    float wl = veml3328_estimate_wavelength(0, 0, 255);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, VEML3328_WAVELENGTH_BLUE, wl);
}

void test_wavelength_black(void) {
    float wl = veml3328_estimate_wavelength(0, 0, 0);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, wl);
}

void test_wavelength_mixed(void) {
    float wl = veml3328_estimate_wavelength(100, 100, 100);
    float expected = (VEML3328_WAVELENGTH_RED + VEML3328_WAVELENGTH_GREEN + VEML3328_WAVELENGTH_BLUE) / 3.0f;
    TEST_ASSERT_FLOAT_WITHIN(0.001f, expected, wl);
}

void setUp(void) {
    // Reset dummy variables before each test
    memset(dummy_written_buf, 0, sizeof(dummy_written_buf));
    memset(dummy_read_buf, 0, sizeof(dummy_read_buf));
    memset(dummy_reg_values, 0, sizeof(dummy_reg_values));

    dummy_written_length = 0;
    dummy_fail = 0;
    current_reg = 0;
}

void tearDown(void) {
    // Nothing to clean up after each test
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_write_reg);
    RUN_TEST(test_read_reg);
    RUN_TEST(test_read_raw);
    RUN_TEST(test_norm);
    RUN_TEST(test_config);
    RUN_TEST(test_wavelength_red_pure);
    RUN_TEST(test_wavelength_green_pure);
    RUN_TEST(test_wavelength_blue_pure);
    RUN_TEST(test_wavelength_black);
    RUN_TEST(test_wavelength_mixed);

    return UNITY_END();
}   