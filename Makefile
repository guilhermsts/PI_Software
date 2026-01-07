CC := gcc
CFLAGS := -Wall -Wextra -I./src -I./tests

SRC_DIR := src
TEST_DIR := tests
BUILD_DIR := build

SRC_TCA   := $(SRC_DIR)/tca9548a.c
SRC_VEML  := $(SRC_DIR)/veml3328.c
TEST_TCA  := $(TEST_DIR)/test_tca.c
TEST_VEML := $(TEST_DIR)/test_veml.c
UNITY     := $(TEST_DIR)/unity.c

# Tests binaries
TEST_VEML_BIN := $(BUILD_DIR)/test_veml
TEST_TCA_BIN  := $(BUILD_DIR)/test_tca

.PHONY: all
# Build both test executables
all: $(TEST_VEML_BIN) $(TEST_TCA_BIN)

# Ensure build dir exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# TCA tests
$(TEST_TCA_BIN): $(BUILD_DIR) $(UNITY) $(SRC_TCA) $(TEST_TCA)
	$(CC) $(CFLAGS) -o $@ $(UNITY) $(TEST_TCA) $(SRC_TCA)

# VEML tests
$(TEST_VEML_BIN): $(BUILD_DIR) $(UNITY) $(TEST_VEML) $(SRC_VEML)
	$(CC) $(CFLAGS) -o $@ $(UNITY) $(TEST_VEML) $(SRC_VEML)

.PHONY: test_veml test_tca test
test_veml: $(TEST_VEML_BIN)
	./$(TEST_VEML_BIN)

test_tca: $(TEST_TCA_BIN)
	./$(TEST_TCA_BIN)

test: test_veml test_tca

# Raspberry Pi specific application build
PI_APP := $(BUILD_DIR)/pi_app
PI_SRC := $(SRC_DIR)/main.c $(SRC_DIR)/i2c_driver_pi.c $(SRC_VEML) $(SRC_TCA)
PI_TEST_SENSOR 	:= $(BUILD_DIR)/test_sensor
PI_TEST_SRC 	:= $(SRC_DIR)/test_sensor.c $(SRC_DIR)/i2c_driver_pi.c $(SRC_VEML) $(SRC_TCA)

.PHONY: pi_app
pi_app: $(BUILD_DIR) $(PI_SRC)
	$(CC) $(CFLAGS) -o $(PI_APP) $(PI_SRC)

.PHONY: pi_test_sensor
pi_test_sensor: $(BUILD_DIR) $(PI_TEST_SRC)
	$(CC) $(CFLAGS) -o $(PI_TEST_SENSOR) $(PI_TEST_SRC)

BRIDGE_SO := $(BUILD_DIR)/sensor_bridge.so
BRIDGE_SRC := $(SRC_DIR)/sensor_bridge.c $(SRC_VEML) $(SRC_TCA) $(SRC_DIR)/i2c_driver_pi.c

.PHONY: bridge
bridge: $(BUILD_DIR) $(BRIDGE_SO)

$(BRIDGE_SO): $(BRIDGE_SRC)
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $(BRIDGE_SRC)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)