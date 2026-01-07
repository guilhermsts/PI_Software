class sensor:
    R = 0
    B = 0
    G = 0
    Intensity = 0
    Wavelength = 0

    def setValues(self,R_v,G_V,B_V,INT_V,WAVE_V):
        self.R=R_v
        self.G=G_V
        self.B=B_V
        self.Intensity=INT_V
        self.Wavelength=WAVE_V


sensor1=sensor()
sensor1.setValues(42,6,30,80,400)
sensor2=sensor()
sensor2.setValues(23,56,84,1,556)
sensor3=sensor()
sensor3.setValues(45,83,92,30,654)
sensor4=sensor()
sensor4.setValues(23,57,342,85,442)
sensor5=sensor()
sensor5.setValues(123,41,64,100,542)
sensor6=sensor()
sensor6.setValues(234,626,134,22,466)
sensor7=sensor()
sensor7.setValues(325,64,23,12,635)
sensor8=sensor()
sensor8.setValues(39,62,24,72,535)


sensor_array=[sensor1,sensor2,sensor3,sensor4,sensor5,sensor6,sensor7,sensor8]