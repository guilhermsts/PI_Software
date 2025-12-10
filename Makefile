CC = gcc
CFLAGS = -Wall -Wextra -Isrc -Itests
BUILD_DIR = build

UNITY     = tests/unity.c
SRC_TCA   = src/tca9548a.c
SRC_VEML  = src/veml3328.c
TEST_TCA  = tests/test_tca.c
TEST_VEML = tests/test_veml.c

# Build both test executables
all: $(BUILD_DIR)/test_tca $(BUILD_DIR)/test_veml

# Ensure build dir exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# TCA tests
$(BUILD_DIR)/test_tca: $(BUILD_DIR) $(UNITY) $(SRC_TCA) $(TEST_TCA)
	$(CC) $(CFLAGS) $(UNITY) $(SRC_TCA) $(TEST_TCA) -o $@

# VEML tests
$(BUILD_DIR)/test_veml: $(BUILD_DIR) $(UNITY) $(SRC_VEML) $(TEST_VEML)
	$(CC) $(CFLAGS) $(UNITY) $(SRC_VEML) $(TEST_VEML) -o $@

clean:
	rm -f $(BUILD_DIR)/test_tca $(BUILD_DIR)/test_veml