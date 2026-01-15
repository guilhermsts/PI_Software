# Project Overview
This project implements a multi-sensor RGB measurement system based on the VEML3328 colour sensor and a TCA9548A I2C multiplexer, which runs on a Raspberry Pi.

# Hardware Requirements
- Raspberry Pi (with I2C communications)
- VEML3328 colour sensors (up to 8)
- TCA9548A I2C multiplexer

# The system provides: 
- RGB values (0-255); 
- Estimated wavelength (nm); 
- Light intensity (uW/(cm^2));

# Measurements are accessed via: 
- REST-API (Python + Flask); 
- Graphical User Interface (GUI);

# Project Structure
- `src/` - Sensor drivers and logic
    - Drivers: `veml3328.c`, `tca9548a.c`, `i2c_driver_pi.c`
    - Applications: `main.c` and `test_sensor.c` (standalone); `sensor_bridge.c` (shared library)
- `tests/` - Unit tests (Unity)
    - Tests: test_tca.c, test_veml.c
- `build/`- Compiled files and shared library
- `GUI/` - GUI files 
- `GUI/API/` - REST API (Python)
- `Makefile` - Build system

In the project was included a Makefile to facilitate the compilation process. There is the possibility of compiling everything at the same time, through the comand "make" in the main folder. Or choosing to compile only a couple of files, to do so (compilation instructions):

```bash
make
    Builds all files:
        >> build/pi_app
        >> build/test_sensor
        >> build/sensor_bridge.so
        >> build/test_tca
        >> build/test_veml

make bridge 
    Builds the shared library for the API: 
        >> build/sensor_bridge.so

make pi_app 
    Builds the satndalone Raspberry pi application:
        >> build/pi_app

make pi_test_sensor 
    Builds the satndalone sensor test application:
        >> build/test_sensor

make test 
    Builds all unit tests:
        >> build/test_tca
        >> build/test_veml

make test_veml 
    Builds only the Veml3328 driver test 
        >> build/test_veml
make test_tca 
    Builds only the Tca9548a driver test
        >> build/test_tca

```

# API

This project has an REST API to ensue comunication between the I2C modules and the user interface.

At the first use of the API in the raspberry these setup steps are required:

```bash
cd GUI/API
python3 -m venv .venv
source .venv/bin/activate
pip install flask flask-restful
```

To start the API it is only necessary to run the `api.py` file in the API directory as shown bellow:

```bash
python api.py
```

Currently, the API has one post method on `http://{raspberry_ip}:5000/read_sensors`, this post receives the selected sensor array and the sensitivity state in JSON format, and returns the data obtained by the sensors, also in JSON format.

In order for the API to work and connect with the I2C the file `sensor_bridge.so` is required in the build folder.

# GUI Usage

The Graphic user interface allows the user to:

- Select the sensors necessary to scan;
- Toggle sensivity, if required;
- Start the simulation;
- Update values and present them in a table and a color graph;
  - For readability when the user hoovers a table line that value is enhanced and labeled on the color graph.

This interface works as long as the device it is being used on is in the same network as the API, in order to set that network the variable `ip_raspberry` needs to be changed to the network's ip address.

# Use Examlpe
To operate the system, the following devices must be connected to the same local network:
- Raspberry P;
- PC accessing the Raspberry Pi via SSH;
- PC running the GUI;

### One-time Raspberry Pi setup
**Enable I2C**
```bash
sudo raspi-config
# Interface Options -> I2C -> Enable
sudo reboot
```
Although I2C can be enabled through the Raspberry Pi desktop interface, `sudo raspi-config` is recommended as it works in headless and remote configurations.

**Install required system packages**
```bash
sudo apt update
sudo apt install -y git build-essential python3 python3-venv
```

### Initial Rapsberry Pi Access
The Raspberry Pi can be accessed remotely via SSH.

To obtain the IP address, run:
```bash
hostname -I
```
Then connect from another machine:
```bash
ssh <username>@<ip_address>
```

The username depends on the operating system configuration. In this project setup, the default user was `raspgui`.
When deploying the system in a production environment, it is recommended that the organisation creates and manages its own user accounts and credentials.

### Project Location
After connecting via SSH, navigate to the project directory:
```bash
cd ~/Desktop/PI_Software
```

### Building the Software
To compile all required binaries and libraries:
```bash
make
```
This will generate the files referred to later.

To compile only the shared library used by the API:
```bash
make bridge
```
**Note**: The shared library `sensor_bridge.so` is loaded automatically by the REST API and does not need to be exectuted manually.

### Running the REST API
Navigate to the API directory, inside the project directory:
```bash
cd GUI/API
```
Create and activate a Python virtual environment:
```bash
python3 -m venv .venv
source .venv/bin/activate
```
Install dependencies (only required once per system):
```bash
pip install flask flask-restful
```
Start the API:
```bash
python api.py
```

The API will be available at `http://<raspberry_ip>:5000`

### Running the GUI
.....