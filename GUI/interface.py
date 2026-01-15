import tkinter
from tkinter import PhotoImage,Toplevel
from functools import partial
from datetime import datetime
import time
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure 
from PIL import Image
import requests
import subprocess
import sys
import atexit
import os
import threading

# iniciar a API
API_COMMAND = [sys.executable, "API/api.py"] 
api_process = None 

def start_api():
    global api_process
    print("A iniciar a API...")

    # Popen para não bloquear a execução do programa principal
    try:
        api_process = subprocess.Popen(
            API_COMMAND,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0 
        )
        print(f"API iniciada com PID: {api_process.pid}")
        #pausa pra dar tempo á api de iniciar
        time.sleep(2) 

    except FileNotFoundError:
        print(f"ERRO: Não foi possível encontrar o ficheiro da API ou o interpretador Python.")
    except Exception as e:
        print(f"ERRO ao iniciar a API: {e}")

def stop_api():

    global api_process
    if api_process is not None:
        if api_process.poll() is None: # Verifica se o processo ainda está em execução
            print(f"A terminar a API (PID: {api_process.pid})...")
            api_process.terminate() # Envia um sinal de terminação 
            try:
                api_process.wait(timeout=5) # Espera que o processo termine
                print("API terminada com sucesso.")
            except subprocess.TimeoutExpired:
                print("Aviso: A API demorou a terminar. A forçar a paragem.")
                api_process.kill()
        else:
            print("API já estava terminada.")

#regista a funçao para quando a api é parada
atexit.register(stop_api) 


class MyGUI:
    
    #variaveis
    sensitivity_state = 0
    sensor_states = [0,0,0,0,0,0,0,0] 
    error_flag = [0,0,0,0,0,0,0,0]

    sensor_plots = {} 
    sensor_texts = {} 

    bgcolor="#CBCBD6"
    fgcolor="#15565F"
    darkcolor="#0a1718"
    point_unselect_color="#44464A" 
    point_select_color="#1F232A" 
    point_edge_color="#CFDCF2"  

    def __init__(self):
    
        #janela---------
        self.window=tkinter.Tk()
        self.window.state('zoomed')
        self.window.title("LED Analyser")
        self.window.configure(bg = self.bgcolor)  #bg color
        self.window.resizable=(False,False)
        #end janela-------

        self.check=PhotoImage(file = r"images\checkbox.png")
        self.uncheck=PhotoImage(file = r"images\unchecked.png")

        self.fig =Figure(figsize=(5, 4), dpi=100, facecolor=self.bgcolor)
        self.spectrogram_plot = self.fig.add_subplot(111)
        self.spectrogram_plot.patch.set_alpha(0.0)

        self.background_img = None
        self.load_background_image("images/graph.png")
        #grafico
        
        #frames------
        self.left_frame = tkinter.Frame(self.window, bg=self.bgcolor)
        self.left_frame.grid(row=1,column=1, padx=(65,35),pady=(45,15))

        self.title_frame = tkinter.Frame(self.left_frame, bg = self.bgcolor)
        self.title_frame.pack()
        self.buttons_frame = tkinter.Frame(self.left_frame, bg = self.bgcolor)
        self.buttons_frame.pack(pady=40,padx=10)
        self.table_frame = tkinter.Frame(self.left_frame, bg = self.bgcolor
                                        ,highlightbackground=self.darkcolor,highlightthickness=4)
        self.table_frame.pack(padx=10)

        self.right_frame = tkinter.Frame(self.window,bg = self.bgcolor)
        self.right_frame.grid(row=1,column=2,padx=(25,15),pady=(45,15))

        self.date_hour_frame = tkinter.Frame(self.right_frame,bg = self.bgcolor,
                                             highlightbackground=self.fgcolor,highlightthickness=6,
                                             padx= 10, pady=10)
        self.date_hour_frame.pack(anchor='n')
        
        self.graph_frame =tkinter.Frame(self.right_frame,bg = self.bgcolor
                                        ,highlightbackground=self.fgcolor,highlightthickness=4)
        self.graph_frame.pack(anchor='n',pady=40)
        #end frames------

        #TITLE
        self.LED_Analyser_label=tkinter.Label(self.title_frame,text="LED Analyser",
                                        bg = self.bgcolor,fg="#000000",
                                        font=("Noto Sans KR Black",42))
        self.LED_Analyser_label.grid(row=1,column=2,sticky="news", pady=20,padx=70)

        
        self.button_start=tkinter.Button(self.buttons_frame, text="START",
                                               command=self.start_sim,
                                               bg=self.fgcolor,fg=self.bgcolor,
                                               font = ("Noto Sans KR Black", 25),
                                               height= 1, width= 15)
        self.button_start.grid(row=0,column=1, pady=10,padx=20)

        self.button_sensitivity=tkinter.Button(self.buttons_frame, text="SENSITIVITY",
                                               command=self.sensitivity_toggle,
                                               bg=self.fgcolor,fg=self.bgcolor,
                                               font = ("Noto Sans KR Black", 25),
                                               height= 1, width= 15,
                                                highlightbackground=self.fgcolor,highlightthickness=4,bd=0)
        self.button_sensitivity.grid(row=0,column=2, pady=10,padx=20)

        
        #-----------

        #Tabela
        #1a linha
        self.checkall_frame = tkinter.Frame(self.table_frame, bg = self.bgcolor,height=3,
                                            highlightbackground=self.darkcolor,highlightthickness=1)
        self.checkall_frame.grid(row=1,column=0)

        self.check_all_sensors=tkinter.Button(self.checkall_frame, command = partial(self.toggle_check,0),
                                            height=80,
                                            padx=10,pady=10,
                                            bg = self.fgcolor,fg = self.bgcolor,
                                            image=self.uncheck,bd=0)
        self.check_all_sensors.pack(anchor="center")

        self.labels_dict = {}
        self.labels_names = [("LED",1),(" R ",2),(" G ",3),(" B ",4),("Intensity",5), ("Wavelength (nm)",6)]
        for name,num in self.labels_names:
        
            self.labels_dict[name] = tkinter.Label(self.table_frame, text=name,
                                            height = 1,
                                            bg = self.fgcolor,fg = self.bgcolor,
                                            font = ("Noto Sans KR Black", 18),
                                            padx=15,pady=15,
                                            highlightbackground=self.darkcolor,highlightthickness=1)
            self.labels_dict[name].grid(row=1,column=num,sticky="news")

        # RESTO DA TABELA
        self.checkbox_dict = {}
        self.checkbox_frame={}
        self.sensor_lable_dict= {}
        self.R_dict= {}
        self.B_dict= {}
        self.G_dict= {}
        self.Intensity_dict= {}
        self.wavelength_dict = {}
        self.checkbox_numbers = [1,2,3,4,5,6,7,8]
        for num in self.checkbox_numbers:
            #checkboxes
            self.checkbox_frame[num] = tkinter.Frame(self.table_frame, bg = self.bgcolor,
                                                    highlightbackground=self.darkcolor,highlightthickness=1,
                                                    pady=5)
            self.checkbox_frame[num].grid(row=num+1,column=0,sticky="news")

            self.checkbox_dict[num] = tkinter.Button(self.checkbox_frame[num], command =  partial(self.toggle_check,num),
                                                image=self.uncheck, bg = self.bgcolor,bd=0)
                                                
            self.checkbox_dict[num].pack(anchor="s")

            #led labels
            self.sensor_lable_dict[num] = tkinter.Label(self.table_frame,text =str(num),
                                                   bg = self.bgcolor,
                                                   font = ("Noto Sans KR Black", 16),
                                                   highlightbackground=self.darkcolor,highlightthickness=1,
                                                   pady=10,padx=10)
            self.sensor_lable_dict[num].grid(row=num+1,column=1,sticky="news")

            #R,G,B
            self.R_dict[num] = tkinter.Label(self.table_frame,text = "-",
                                                   bg = self.bgcolor,
                                                   font = ("Noto Sans KR Black", 16),
                                                   highlightbackground=self.darkcolor,highlightthickness=1,
                                                   pady=10,padx=10)
            self.R_dict[num].grid(row=num+1,column=2,sticky="news")

            self.G_dict[num] = tkinter.Label(self.table_frame,text = "-",
                                                   bg = self.bgcolor,
                                                   font = ("Noto Sans KR Black", 16),
                                                   highlightbackground=self.darkcolor,highlightthickness=1,
                                                   pady=10,padx=10)
            self.G_dict[num].grid(row=num+1,column=3,sticky="news")

            self.B_dict[num] = tkinter.Label(self.table_frame,text = "-",
                                                   bg = self.bgcolor,
                                                   font = ("Noto Sans KR Black", 16),
                                                   highlightbackground=self.darkcolor,highlightthickness=1,
                                                   pady=10,padx=10)
            self.B_dict[num].grid(row=num+1,column=4,sticky="news")

            #Intensidade Wavelength
            self.Intensity_dict[num] = tkinter.Label(self.table_frame,text = "-",
                                                   bg = self.bgcolor,
                                                   font = ("Noto Sans KR Black", 16),
                                                   highlightbackground=self.darkcolor,highlightthickness=1,
                                                   pady=10,padx=10)
            self.Intensity_dict[num].grid(row=num+1,column=5,sticky="news")

            self.wavelength_dict[num] = tkinter.Label(self.table_frame,text = "-",
                                                   bg = self.bgcolor,
                                                   font = ("Noto Sans KR Black", 16),
                                                   highlightbackground=self.darkcolor,highlightthickness=1,
                                                   pady=10,padx=10)
            self.wavelength_dict[num].grid(row=num+1,column=6,sticky="news")

            #fazer o binding para o hover do rato
            widgets_da_linha = [
                self.sensor_lable_dict[num], 
                self.R_dict[num], 
                self.G_dict[num], 
                self.B_dict[num], 
                self.Intensity_dict[num], 
                self.wavelength_dict[num],
                self.checkbox_frame[num]
            ]

            for w in widgets_da_linha:
                w.bind("<Enter>", partial(self.highlight_point, num))
                w.bind("<Leave>", partial(self.unhighlight_point, num))

        #Data e hora-------
        self.date_hour_label=tkinter.Label(self.date_hour_frame,text="Date and Time",
                                         bg = self.bgcolor,fg="#000000",
                                         font = ("Noto Sans KR Black", 38),)
        self.date_hour_label.grid(row=1,column=0, pady=10,padx=10)
        
        now = datetime.now()
      
        self.date_label=tkinter.Label(self.date_hour_frame,text=now.strftime("%d/%m/%Y"),
                                         bg = self.bgcolor,fg="#000000",
                                         font = ("Noto Sans KR Black", 32),)
        
        self.date_label.grid(row=2,column=0, pady=10,padx=10)

        self.hour_label=tkinter.Label(self.date_hour_frame,text=now.strftime("%H:%M:%S"),
                                         bg = self.bgcolor,fg="#000000",
                                        font = ("Noto Sans KR Black", 32))
        
        self.hour_label.grid(row=3,column=0, pady=10,padx=10)
        #----------

        #grafico
        self.fig = Figure(figsize=(5, 4), dpi=135, facecolor=self.bgcolor)
        self.spectrogram_plot = self.fig.add_subplot(111)
        
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.graph_frame)
        self.canvas_widget = self.canvas.get_tk_widget()

        self.spectrogram_plot.imshow(
            self.background_img, 
            aspect='auto',        # Ajustar a imagem para preencher o espaço do subplot
            extent=[400, 720, 0, 10], # Define a área (xmin, xmax, ymin, ymax) onde a imagem aparece. 
            zorder=0,               # Zorder 0 (por baixo das linhas do gráfico)
            alpha=0.7)

        self.canvas_widget.pack(fill=tkinter.BOTH, expand=True)
        self.spectrogram_plot.set_title('Light Spectrum of the selected LEDs', fontsize=12, color=self.fgcolor)
        self.spectrogram_plot.set_xlabel('Wavelength (nm)', fontsize=10)
        self.spectrogram_plot.set_ylabel('Relative Intensity', fontsize=10)
        self.spectrogram_plot.tick_params(axis='x', colors=self.darkcolor)
        self.spectrogram_plot.tick_params(axis='y', colors=self.darkcolor)
        self.canvas.draw()
        #----

        self.window.protocol("WM_DELETE_WINDOW",self.on_closing)

        self.window.mainloop()
    #----------------    

    def load_background_image(self, image_path):
    #"""Carrega a imagem de fundo e ajusta a transparência/tamanho se necessário."""
        try:
            # Abrir a imagem
            img = Image.open(image_path)

            target_size = (self.fig.get_size_inches() * self.fig.dpi).astype(int)
            img = img.resize(target_size) 
            
            self.background_img = img

        except FileNotFoundError:
            print(f"Aviso: Ficheiro de imagem de fundo não encontrado em: {image_path}")
            self.background_img = None
        except Exception as e:
            print(f"Erro ao carregar imagem de fundo: {e}")
            self.background_img = None

    def on_closing(self):
        self.window.destroy()

    def toggle_check(self,number):
        match number:
            case 0:
                #descelecionar tudo
                if all([x == 1 for x in self.sensor_states]):
                    self.check_all_sensors.config(image=self.uncheck)  
                    for n in self.checkbox_numbers:
                        self.checkbox_dict[n].config(image=self.uncheck) 
                        self.sensor_states[n-1]=0
                        pass
                else:
                    self.check_all_sensors.config(image=self.check)  
                    for n in self.checkbox_numbers:
        
                        self.checkbox_dict[n].config(image=self.check)  
                        self.sensor_states[n-1]=1
                        pass
            case 1:
                if self.sensor_states[0]:
                    self.checkbox_dict[1].config(image=self.uncheck)
                    self.sensor_states[0]=0
                else:
                    self.checkbox_dict[1].config(image=self.check)
                    self.sensor_states[0]=1
            case 2:
                if self.sensor_states[1]:
                    self.checkbox_dict[2].config(image=self.uncheck)
                    self.sensor_states[1]=0
                else:
                    self.checkbox_dict[2].config(image=self.check)
                    self.sensor_states[1]=1
            case 3:
                if self.sensor_states[2]:
                    self.checkbox_dict[3].config(image=self.uncheck)
                    self.sensor_states[2]=0
                else:
                    self.checkbox_dict[3].config(image=self.check)
                    self.sensor_states[2]=1
            case 4:
                if self.sensor_states[3]:
                    self.checkbox_dict[4].config(image=self.uncheck)
                    self.sensor_states[3]=0
                else:
                    self.checkbox_dict[4].config(image=self.check)
                    self.sensor_states[3]=1
            case 5:
                if self.sensor_states[4]:
                    self.checkbox_dict[5].config(image=self.uncheck)
                    self.sensor_states[4]=0
                else:
                    self.checkbox_dict[5].config(image=self.check)
                    self.sensor_states[4]=1
            case 6:
                if self.sensor_states[5]:
                    self.checkbox_dict[6].config(image=self.uncheck)
                    self.sensor_states[5]=0
                else:
                    self.checkbox_dict[6].config(image=self.check)
                    self.sensor_states[5]=1
            case 7:
                if self.sensor_states[6]:
                    self.checkbox_dict[7].config(image=self.uncheck)
                    self.sensor_states[6]=0
                else:
                    self.checkbox_dict[7].config(image=self.check)
                    self.sensor_states[6]=1
            case 8:
                if self.sensor_states[7]:
                    self.checkbox_dict[8].config(image=self.uncheck)
                    self.sensor_states[7]=0
                else:
                    self.checkbox_dict[8].config(image=self.check)
                    self.sensor_states[7]=1
            case _:
                pass
        
        if all([x == 1 for x in self.sensor_states]):
            self.check_all_sensors.config(image=self.check)  
        else:
            self.check_all_sensors.config(image=self.uncheck)  
    #-----end checkboxes                    

    def start_sim(self):
            
        now = datetime.now()
      
        self.date_label.config(text=now.strftime("%d/%m/%Y"))
        self.hour_label.config(text = now.strftime("%H:%M:%S"))

        ip_raspberry = "10.54.117.64" 
        url = f"http://{ip_raspberry}:5000/read_sensors"

        data = {
            "sensors": self.sensor_states,
            "sensitivity": self.sensitivity_state
            }
        
        self.waiting_window("the simulation \nis running ",url,data)

    def process_results(self,response, window_to_close):
        if response.status_code == 200:
            sensor_array = response.json()

        self.error_flag = [0,0,0,0,0,0,0,0]
        
        i=0
        max_int=10
        
        for i in range(len(self.sensor_states)):
            if self.sensor_states[i] == 1 :
                self.R_dict[i+1].config(text = str(round(sensor_array[i]["R"])))
                self.G_dict[i+1].config(text = str(round(sensor_array[i]["G"])))
                self.B_dict[i+1].config(text = str(round(sensor_array[i]["B"])))
                self.Intensity_dict[i+1].config(text = str(round(sensor_array[i]["Intensity"],2)))
                self.wavelength_dict[i+1].config(text = str(round(sensor_array[i]["Wavelength"])))

                #dar erro se medir um comprimento de onda fora do limite de visibilidade
                if(sensor_array[i]["Wavelength"] > 399 and sensor_array[i]["Wavelength"] < 701):
                    if sensor_array[i]["Intensity"] > max_int:
                            max_int = sensor_array[i]["Intensity"]
                else:
                    self.wavelength_dict[i+1].config(text = "ERROR")
                    self.error_flag[i] = 1

            else:
                self.R_dict[i+1].config(text = str(sensor_array[i]["R"]))
                self.G_dict[i+1].config(text = str(sensor_array[i]["G"]))
                self.B_dict[i+1].config(text = str(sensor_array[i]["B"]))
                self.Intensity_dict[i+1].config(text = str(sensor_array[i]["Intensity"]))
                self.wavelength_dict[i+1].config(text = str(sensor_array[i]["Wavelength"]))


        #limpar o espectrograma
        self.spectrogram_plot.clear()

        self.spectrogram_plot.imshow(
            self.background_img, 
            aspect='auto',        # Ajustar a imagem para preencher o espaço do subplot
            extent=[400, 700, 0, max_int+2], # Define a área (xmin, xmax, ymin, ymax) onde a imagem aparece. 
            zorder=0,               # Zorder 0 (por baixo das linhas do gráfico)
            alpha=0.7)
            
        # Percorrer os dados de todos os sensores
        for i in range(len(self.sensor_states)):
            # Verificar se o sensor está selecionado E tem dados 
            if self.sensor_states[i] == 1:
                #não desenhar quando o wavelength estiver fora do espectro visivel
                if(sensor_array[i]["Wavelength"] < 400 or sensor_array[i]["Wavelength"] > 700):
                    continue

                sensor_data = sensor_array[i] 
                #Plotar o espectro 
                point = self.spectrogram_plot.scatter(sensor_data["Wavelength"],sensor_data["Intensity"], 
                                        linewidth=2, color = "#1F232A")
                self.sensor_plots[i+1] = point 

                #texto
                txt = self.spectrogram_plot.text(
                    sensor_data["Wavelength"], sensor_data["Intensity"] + (max_int * 0.05),str(i+1),
                    fontsize=12, fontweight='bold',ha='center',va='bottom',
                    color=self.darkcolor,zorder=11,
                    bbox=dict(facecolor='white', alpha=0.7, edgecolor='none', boxstyle='round,pad=0.2')
                )                
                txt.set_visible(False) 
                self.sensor_texts[i+1] = txt


                #Configurar o gráfico
                self.spectrogram_plot.set_title('Light Spectrum of the selected LEDs', fontsize=12, color=self.fgcolor)
                self.spectrogram_plot.set_xlabel('Wavelength (nm)', fontsize=10)
                self.spectrogram_plot.set_ylabel('Relative Intensity', fontsize=10)
                self.spectrogram_plot.tick_params(axis='x', colors=self.darkcolor)
                self.spectrogram_plot.tick_params(axis='y', colors=self.darkcolor)
                self.spectrogram_plot.grid(True, linestyle='--', alpha=0.6,color=self.fgcolor)
       
                self.canvas.draw()

        window_to_close.after(0, window_to_close.destroy)

    #funções para mexer no gráfico
    def highlight_point(self, sensor_num, event):
       print(sensor_num)
       if sensor_num in self.sensor_plots:
        # Muda a cor e aumenta o tamanho
        self.sensor_plots[sensor_num].set_facecolor(self.point_select_color)
        self.sensor_plots[sensor_num].set_edgecolor(self.point_edge_color)
        self.sensor_plots[sensor_num].set_sizes([200]) # Ponto maior
        self.sensor_plots[sensor_num].set_zorder(10)   # Traz para a frente

        if sensor_num in self.sensor_texts:
            self.sensor_texts[sensor_num].set_visible(True)

        self.canvas.draw_idle() # Atualiza o gráfico

    def unhighlight_point(self, sensor_num, event):
        if sensor_num in self.sensor_plots:
            # Volta ao normal
            self.sensor_plots[sensor_num].set_facecolor(self.point_unselect_color)
            self.sensor_plots[sensor_num].set_edgecolor(self.point_unselect_color)
            self.sensor_plots[sensor_num].set_sizes([50])
            self.sensor_plots[sensor_num].set_zorder(3)

            # ESCONDER o número
        if sensor_num in self.sensor_texts:
            self.sensor_texts[sensor_num].set_visible(False)

            self.canvas.draw_idle()

    #botão da sensibilidade
    def sensitivity_toggle(self):
        if self.sensitivity_state: 
            self.button_sensitivity.config(bg = self.fgcolor,fg = self.bgcolor)
            self.sensitivity_state = 0
        else:
            self.button_sensitivity.config(bg="#3A3A3E",fg="#000000")
            self.sensitivity_state = 1

    #----------~
    #abrir nova janela
    def waiting_window(self,message,url,data):
        new_window = Toplevel(self.window)  # Create a new window
        new_window.title("Running")
        new_window.geometry('%dx%d+%d+%d' % (250, 150, 720, 520))  

        tkinter.Label(new_window, text=message,font=("arial",20)).pack(pady=20)

        def fazer_pedido():
        # Isto corre "ao lado" do programa principal
            response = requests.post(url, json=data)
            self.window.after(0, self.process_results, response, new_window)
           
        # Cria e inicia a thread
        thread = threading.Thread(target=fazer_pedido)
        thread.start()
            
        

#-------------

MyGUI()