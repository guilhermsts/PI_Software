from flask import Flask, request, jsonify
import ctypes
import os
from flask_restful import Api

from test_sensor_data import sensor_array


app = Flask(__name__)
api= Api(app)   #definir a api

# Definir a classe correspondente à struct(?) dos resultados
class SensorResult(ctypes.Structure):
    _fields_ = [
        ("R", ctypes.c_float),
        ("B", ctypes.c_float),
        ("G", ctypes.c_float),
        ("Intensity", ctypes.c_float),
        ("Wavelength", ctypes.c_float)
    ]

#gcc -shared -o libsensors.dll test.c -m64, compilar o código em C com isto 
#(feel free de compor o caminho e assim só é preciso haver um .so ou um .dll)
#
#lib_path = os.path.abspath(r"SW_CONTROLO\libsensors.dll") 
#lib = ctypes.CDLL(lib_path)


HERE = os.path.dirname(os.path.abspath(__file__))
lib_path = os.path.join(HERE, "..", "build", "sensor_bridge.so")
lib = ctypes.CDLL(lib_path)

# Define os tipos de entrada e saída da função C
lib.get_sensor_readings.argtypes = [ctypes.c_int, ctypes.c_int]
lib.get_sensor_readings.restype = SensorResult

#oque vai na mensagem do gui para o controlador
class SensorRequest():
    sensors: list[int]
    sensitivity: int 

@app.post("/read_sensors")
def read_sensors():
    
    data = request.get_json()

    sensors = data.get("sensors")
    sensitivity = data.get("sensitivity")
    
    sensor_list=[]

    print(sensor_array)
    for i in range(8):
        
        #inicializar o vetors
        sen_data = [1,2,3,4,5]

        if sensors[i]:
            #ler os sensores
            sensor_result=lib.get_sensor_readings(i,int(sensitivity))

            sen_data[0] = sensor_result.R
            sen_data[1] = sensor_result.B
            sen_data[2] = sensor_result.G
            sen_data[3] = sensor_result.Intensity
            sen_data[4] = sensor_result.Wavelength

        else:
            sen_data[0]= "-"
            sen_data[1]= "-"
            sen_data[2]= "-"
            sen_data[3]= "-"
            sen_data[4]= "-"
            
        sensor = {
        "number" : i+1,
        "R" : sen_data[0],
        "B" : sen_data[1],
        "G" : sen_data[2],
        "Intensity" : sen_data[3],
        "Wavelength" : sen_data[4]
        }
        
        sensor_list.append(sensor)

    
    return jsonify(sensor_list)

@app.post('/')
def home():    
    return f"<a>"


if __name__ == '__main__':
    app.run(debug=True)