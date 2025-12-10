#include "unity.h"
#include "../src/tca9548a.h"
#include <stdint.h>

/* Dummy functions for i2c backend tests*/
static int dummy_i2c_fd;
static uint8_t dummy_last_dev_addr;
static uint8_t dummy_last_data;
static int dummy_fail_write;

int i2c_write_byte(int i2c_fd, uint8_t dev_addr_7bit, uint8_t data) {
    dummy_i2c_fd = i2c_fd;
    dummy_last_dev_addr = dev_addr_7bit;
    dummy_last_data = data;
    if (dummy_fail_write) {
        return -1;  // Simulate failure
    }
    return 0;       // Simulate success
}

int i2c_read_byte(int i2c_fd, uint8_t dev_addr_7bit, uint8_t *out) {
    if (out == NULL)
    {
        return -1;
    }
    dummy_i2c_fd = i2c_fd;
    dummy_last_dev_addr = dev_addr_7bit;
    *out = dummy_last_data; // Return last written value
    if (dummy_fail_write) {
        return -1;  // Simulate failure
    }
    return 0; // Not used in these tests
}


/* Tests */
void test_channel(void) {
    TEST_ASSERT_EQUAL_HEX8(0X01, tca_encode_channel(0));
    TEST_ASSERT_EQUAL_HEX8(0X02, tca_encode_channel(1));
    TEST_ASSERT_EQUAL_HEX8(0X04, tca_encode_channel(2));
    TEST_ASSERT_EQUAL_HEX8(0X08, tca_encode_channel(3));
    TEST_ASSERT_EQUAL_HEX8(0X10, tca_encode_channel(4));
    TEST_ASSERT_EQUAL_HEX8(0X20, tca_encode_channel(5));
    TEST_ASSERT_EQUAL_HEX8(0X40, tca_encode_channel(6));
    TEST_ASSERT_EQUAL_HEX8(0X80, tca_encode_channel(7));
    TEST_ASSERT_EQUAL_HEX8(TCA_INVALID_CHANNEL, tca_encode_channel(-1));
    TEST_ASSERT_EQUAL_HEX8(TCA_INVALID_CHANNEL, tca_encode_channel(8));
}

void test_tca_write_control(void) {
    dummy_fail_write = 0;
    int ret = tca_write_control(42, 0x70, 0X20);
    TEST_ASSERT_EQUAL_INT(TCA_OK, ret);
    TEST_ASSERT_EQUAL_INT(42, dummy_i2c_fd);
    TEST_ASSERT_EQUAL_HEX8(0x70, dummy_last_dev_addr);
    TEST_ASSERT_EQUAL_HEX8(0X20, dummy_last_data);
}

void test_tca_select_channel(void) {
    dummy_fail_write = 0;
    int ret = tca_select_channel(7, 0x71, 5);   // fd=7, addr=0x71, channel=5 -> 0x20
    TEST_ASSERT_EQUAL_INT(TCA_OK, ret);
    TEST_ASSERT_EQUAL_HEX8(0X20, dummy_last_data);
}

void test_DisableAll(void) {
    dummy_fail_write = 0;
    int ret = tca_disable_all(9, 0x70);   // fd=9, addr=0x70 -> 0x00
    TEST_ASSERT_EQUAL_INT(TCA_OK, ret);
    TEST_ASSERT_EQUAL_HEX8(0x00, dummy_last_data);
}

void test_i2c_write_failure(void) {
    dummy_fail_write = 1;
    int ret = tca_select_channel(1, 0x70, 2);
    TEST_ASSERT_EQUAL_INT(TCA_ERR_I2C_WRITE, ret);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_channel);
    RUN_TEST(test_tca_write_control);
    RUN_TEST(test_tca_select_channel);
    RUN_TEST(test_DisableAll);
    RUN_TEST(test_i2c_write_failure);

    return UNITY_END();
}