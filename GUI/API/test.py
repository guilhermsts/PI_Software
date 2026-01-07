import requests
import json
import subprocess
import sys
import atexit
import time
import os

# --- Configuração ---
# O comando exato para iniciar a sua API
API_COMMAND = [sys.executable, "API/api.py"] 
# sys.executable garante que usa o mesmo interpretador Python
# 'api_server.py' deve ser o nome do ficheiro da sua API

# Variável para guardar o objeto do processo da API
api_process = None 

# --- Funções de Controlo do Processo ---

def start_api():
    """Inicia a API num novo processo."""
    global api_process

    # Usamos Popen para não bloquear a execução do programa principal
    # shell=False é mais seguro
    # stdout e stderr redirecionados para o pipe para evitar poluir a consola da GUI
    try:
        api_process = subprocess.Popen(
            API_COMMAND,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            creationflags=subprocess.CREATE_NEW_CONSOLE if os.name == 'nt' else 0 
            # O 'creationflags' só é relevante no Windows para abrir numa nova janela
        )
        print(f"✅ API iniciada com PID: {api_process.pid}")
        # Opcional: Pausa breve para dar tempo à API para inicializar a porta
        time.sleep(2) 
    except FileNotFoundError:
        print(f"ERRO: Não foi possível encontrar o ficheiro da API ou o interpretador Python.")
    except Exception as e:
        print(f"ERRO ao iniciar a API: {e}")

def stop_api():
    """Termina o processo da API quando a interface é fechada."""
    global api_process
    if api_process is not None:
        if api_process.poll() is None: # Verifica se o processo ainda está em execução
            api_process.terminate() # Envia um sinal de terminação (SIGTERM no Linux/macOS)
            # api_process.kill() # Alternativa mais agressiva se 'terminate' não funcionar
            try:
                api_process.wait(timeout=5) # Espera que o processo termine
                print("API terminada com sucesso.")
            except subprocess.TimeoutExpired:
                print("Aviso: A API demorou a terminar. A forçar a paragem.")
                api_process.kill()
        else:
            print("API já estava terminada.")

# --- Lógica de Inicialização/Finalização ---

# Regista a função para ser executada quando o programa principal terminar
atexit.register(stop_api) 

# ----------------------------------------------
# Ponto onde a API é chamada antes da GUI
# ----------------------------------------------
start_api()

url = "http://localhost:5000/read_sensors"

data = {
    "sensors": [1, 0, 1, 1, 0, 1, 0, 1],
    "sensitivity": True
}

response = requests.post(url, json=data)

print("STATUS:", response.status_code)
sensors=response.json()
#sensor_dict = json.loads(sensors)
for i in range(8):
    print(sensors[i],"\n")
