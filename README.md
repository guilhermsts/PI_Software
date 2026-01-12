Project Overview
    This project implements a multi-sensor RGB measurement system based on the VEML3328 colour sensor and a TCA9548A I2C multiplexer, which runs on a Raspberry Pi.

Hardware Requirements
    Raspberry Pi (with I2C communications);
    VEML3328 colour sensors (up to 8);
    TCA9548A I2C multiplexer;

The system provides: 
    RGB values (0-255); 
    Estimated wavelength (nm); 
    Light intensity (uW/(cm^2));

Measurements are accessed via: 
    REST-API (Python + Flask); 
    Graphical User Interface (GUI);

Project Structure
    src/        Sensor drivers and logic
        - Drivers: veml3328.c, tca9548a.c, i2c_driver_pi.c
        - Applications: main.c and test_sensor.c (standalone tests, without GUI and API); sensor_bridge.c (final app, shared library)
    tests/      Unit tests (Unity)
        - Tests: test_tca.c, test_veml.c
    build/      Compiled files and shared library
    GUI/        GUI files 
    GUI/API/    REST API (Python)
    Makefile    Build system

In the project was included a Makefile to facilitate the compilation process. There is the possibility of compiling everything at the same time, through the comand "make" in the main folder. Or choosing to compile only a couple of files, to do so (compilation instructions):

>> make
    Builds all files:
        >> build/pi_app
        >> build/test_sensor
        >> build/sensor_bridge.so
        >> build/test_tca
        >> build/test_veml

>> make bridge 
    Builds the shared library for the API: 
        >> build/sensor_bridge.so

>> make pi_app 
    Builds the satndalone Raspberry pi application:
        >> build/pi_app

>> make pi_test_sensor 
    Builds the satndalone sensor test application:
        >> build/test_sensor

>> make test 
    Builds all unit tests:
        >> build/test_tca
        >> build/test_veml

>> make test_veml 
    Builds only the Veml3328 driver test 
        >> build/test_veml
>> make test_tca 
    Builds only the Tca9548a driver test
        >> build/test_tca

API
Running the API
>> cd GUI/API
>> python3 -m venv .venv
>> source .venv/bin/activate
>> pip install flask flask-restful (at the first time of using the Raspberry) 
>> python api.py

GUI Usage
    Select sensors;
    Toggle sensivity, if required;
    Press Start;
    Values update and presentation;