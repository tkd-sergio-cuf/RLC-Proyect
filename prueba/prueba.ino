
// PANTALLA
#include <Wire.h>
#include "HT_SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#ifdef Wireless_Stick_V3
SSD1306Wire  pantalla(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED); // addr , freq , i2c group , resolution , rst
#else
SSD1306Wire  pantalla(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst
#endif

//LORA
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <CayenneLPP.h>
CayenneLPP lpp(51);

///////////////////////////////////////////////////////

hw_timer_t * timer1 = NULL;
hw_timer_t * timer2 = NULL;

//interrupciones
volatile bool interrupcionTimer1 = 0;
volatile bool interrupcionTimer2 = 0;
volatile bool interrupcionModoLora = 0;
volatile bool inactividad, enviarLora = 0;

// Estado
volatile bool modoLora = 0; 

// Pines
int pinMedicionCapacitancia = 32; // Señal generada osilador diseñado para C
int pinMedicionInductancia = 33; // Señal generada osilador diseñado para L
int pinMedicionR = 36;
int pinModoR1 = 13; // Perilla posision 1k - 18.2k 
int pinModoR2 = 17; // Perilla posision 18.2k - 330k  
int pinModoC = 12; // Perilla posision C 
int pinModoL = 2; // Perilla posision L
int pinMultiplexorR1 = 23; // Multiplexor activa modo R1 (1k - 18.2k)
int pinMultiplexorR2 = 3; // Multiplexor activa modo R1 (18.2k - 330k)
int pinInterrupcionBoton = 1; // Funciona interruptor modo lora

//Variables 
float f, variableMedido;
int R1 = 0, R2 = 0, L = 0, C = 0, Vanalog, cnt=0;
int salida[2] = {0 , 0};
struct data{
      String modo; 
      float medicion;
   } ;
data registroDatos[200];
data registroTemp;
String parteEntera = "x", parteDecimal = "x";
String unidad = "", medicion = "", modo = "";
unsigned int periodo = 0; // tiempo en microsegundos

//Conexion lora 
uint32_t appTxDutyCycle = 1500;
uint8_t devEui[] = {0x60, 0x81, 0xF9, 0xE1, 0x26, 0xA0, 0x3A, 0x56};
uint8_t appEui[] = {0x60, 0x81, 0xF9, 0x62, 0xE9, 0xE5, 0xF5, 0x00};
uint8_t appKey[] = {0x34, 0x0D, 0x55, 0xDC, 0x4E, 0x69, 0xA7, 0x4C, 0xC6, 0xAA, 0x0E, 0x29, 0x02, 0x1D, 0x05, 0xC4};
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda,0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef,0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;
uint16_t userChannelsMask[6]={ 0xFF00,0x0000,0x0000,0x0000,0x0000,0x0000 };
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
DeviceClass_t  loraWanClass = CLASS_A;
bool overTheAirActivation = true;
bool loraWanAdr = false;
bool isTxConfirmed = false;
uint8_t appPort = 2;
uint8_t confirmedNbTrials = 4;

static void prepareTxFrame( uint8_t port, float medicion ){
    float x = 0;
    numerosSalida(medicion, salida);
    if(salida[1] >=10){
      x = salida[0] + salida[1]/100;
    }else{  
      x = salida[0] + salida[1]/10;
    }
    CayenneLPP lpp(LORAWAN_APP_DATA_MAX_SIZE);
    lpp.addAnalogInput(1, x);
    lpp.getBuffer(), 
    appDataSize = lpp.getSize();
    memcpy(appData,lpp.getBuffer(),appDataSize);
}

// ISR
void IRAM_ATTR onTimer1() {
  interrupcionTimer1 = 1;
}

void IRAM_ATTR onTimer2() {
  interrupcionTimer2 = 1;
}

void IRAM_ATTR isr() {

  interrupcionModoLora = 1;

}

void setup() {
  Serial.begin(9600);
  //pantalla
  pantalla.init();
  pantalla.clear();
  pantalla.flipScreenVertically();
  pantalla.setFont(ArialMT_Plain_16);
  pantalla.setTextAlignment(TEXT_ALIGN_CENTER);
  //------
  pinMode(pinMedicionCapacitancia, INPUT);
  pinMode(pinMedicionInductancia, INPUT);
  pinMode(pinModoR1, INPUT_PULLUP);
  pinMode(pinModoR2, INPUT_PULLUP);
  pinMode(pinModoC, INPUT_PULLUP);
  pinMode(pinModoL, INPUT_PULLUP);

  pinMode(pinMultiplexorR1, OUTPUT);
  pinMode(pinMultiplexorR2, OUTPUT);

  pinMode(pinInterrupcionBoton, INPUT_PULLDOWN);
  attachInterrupt(pinInterrupcionBoton, isr, CHANGE); // interrupcion exti

  timer1 = timerBegin(0, 64000, true);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 75000, true);


  //timerAlarmEnable(timer1); // interrupcion timer

  timer2 = timerBegin(1, 320, true);                              // El mayor numero preescalador por el cual 80000000 % x == 0. Es 64000 a 0.3s                                                                // Con lo anterior la frecuencia es 1250                                                            //Por lo que va a contar uno cada 800us. Para completar un minuto se necesita 75000 repitiendo ese ciclo
  timerAttachInterrupt(timer2, &onTimer2, true);
  timerAlarmWrite(timer2, 75000, true);

  //timerAlarmEnable(timer2); // interrupcion timer


  modoLora = digitalRead(pinInterrupcionBoton);

  Mcu.begin();
  deviceState = DEVICE_STATE_INIT;

  if(modoLora == 1){
    timerAlarmEnable(timer1);
    timerAlarmDisable(timer2);
  }else{
    timerAlarmEnable(timer2);
    timerAlarmDisable(timer1);
  }
  analogReadResolution(12);
}
 
void loop() {

  if(interrupcionModoLora){
    modoLora = modoLora ? 0 : 1; //toggle en el estado
    interrupcionModoLora = 0;

    if(modoLora == 1){
      timerAlarmEnable(timer1);
      timerAlarmDisable(timer2);
      pantalla.clear();
      pantalla.display();
    }else{
      timerAlarmEnable(timer2);
      timerAlarmDisable(timer1);
    }
  }

  if(interrupcionTimer1){
    if(modoLora == 1){
      medirYenviar();
    }
    interrupcionTimer1 = 0;
  } 

  if(interrupcionTimer2){
    if(modoLora == 0){
      mediYmostrar();
    }
    interrupcionTimer2 = 0;
  }  

  if(enviarLora){
    switch( deviceState ){
    case DEVICE_STATE_INIT:{
      #if(LORAWAN_DEVEUI_AUTO)
            LoRaWAN.generateDeveuiByChipID();
      #endif
            LoRaWAN.init(loraWanClass,loraWanRegion);
            break;
    }
    case DEVICE_STATE_JOIN:{
      LoRaWAN.join();
      break;
    }case DEVICE_STATE_SEND:{
      prepareTxFrame( appPort, variableMedido );
      LoRaWAN.send();
      cnt++;
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }case DEVICE_STATE_CYCLE:{
      txDutyCycleTime = appTxDutyCycle + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
      LoRaWAN.cycle(txDutyCycleTime);
      enviarLora = 0;
      break;
    }case DEVICE_STATE_SLEEP:{
      LoRaWAN.sleep(loraWanClass);
      break;
    }default:{
      deviceState = DEVICE_STATE_INIT;
      break;
    }
  }
  }
}

void medirYenviar(){
  R1 = digitalRead(pinModoR1);
  R2 = digitalRead(pinModoR2);
  L = digitalRead(pinModoL);
  C = digitalRead(pinModoC);

  if(!R1 && R2 && L && C){
    digitalWrite(pinMultiplexorR1, 1);
    digitalWrite(pinMultiplexorR2, 0);
    Vanalog = analogReadMilliVolts(pinMedicionR);
    variableMedido = 3.01926 - (Vanalog / 1059.8);
    variableMedido = variableMedido / 0.00016349;
  }else if(R1 && !R2 && L && C){
    digitalWrite(pinMultiplexorR1, 0);
    digitalWrite(pinMultiplexorR2, 1);
    Vanalog = analogReadMilliVolts(pinMedicionR);
    variableMedido = 3.019260484 - (Vanalog / 1059.82906);
    variableMedido = variableMedido / 0.000009;
  }else if(R1 && R2 && !L && C){
    periodo = pulseIn(pinMedicionInductancia, HIGH);
    f = 1000000/(periodo * 2);
    variableMedido = 230275.4174/(pow(f,2));
  }else if(R1 && R2 && L && !C){
    periodo = pulseIn(pinMedicionCapacitancia, HIGH);
    f = 1000000/(periodo * 2);
    variableMedido = 1/(138629.43*f);
  }else{    
    variableMedido = 0;
  }
  if(cnt>0){deviceState = DEVICE_STATE_SEND;}
  enviarLora = 1;
}

void mediYmostrar(){
  R1 = digitalRead(pinModoR1);
  R2 = digitalRead(pinModoR2);
  L = digitalRead(pinModoL);
  C = digitalRead(pinModoC);
  pantalla.clear();

  if(!R1 && R2 && L && C){
    modo = "r1";
    digitalWrite(pinMultiplexorR1, 1);
    digitalWrite(pinMultiplexorR2, 0);
    Vanalog = analogReadMilliVolts(pinMedicionR);
    variableMedido = 3.01926 - (Vanalog / 1059.8);
    variableMedido = variableMedido / 0.00016349;
    dentroFueraRango(1000 * 0.95, 18165 * 1.05, variableMedido, modo[0]);
  }else if(R1 && !R2 && L && C){
    modo = "r2";
    digitalWrite(pinMultiplexorR1, 0);
    digitalWrite(pinMultiplexorR2, 1);
    Vanalog = analogReadMilliVolts(pinMedicionR);
    variableMedido = 3.019260484 - (Vanalog / 1059.82906);
    variableMedido = variableMedido / 0.000009;
    dentroFueraRango(18165 * 0.95, 330000 * 1.05, variableMedido, modo[0]);
  }else if(R1 && R2 && !L && C){
    modo = "l";
    periodo = pulseIn(pinMedicionInductancia, HIGH);
    f = 1000000/(periodo * 2);
    variableMedido = 230275.4174/(pow(f,2));
    dentroFueraRango(0.001*0.95, 0.330*1.05, variableMedido, modo[0]);
  }else if(R1 && R2 && L && !C){
    modo = "c";
    periodo = pulseIn(pinMedicionCapacitancia, HIGH);
    f = 1000000/(periodo * 2);
    variableMedido = 1/(138629.43*f);
    dentroFueraRango(0.000000001*0.95, 0.00001*1.05, variableMedido, modo[0]);
  }else{    
    modo = "n";
    variableMedido = 0;
    mostrarPantalla(modo[0]);
  }

  registroTemp.medicion = variableMedido;
  registroTemp.modo = modo;
  registrarMedicion(registroTemp, registroDatos);

  verificarInactividad(registroDatos);
  
  pantalla.display();
}

void numerosSalida(float medicion, int* arr){

  float temp = medicion, temp2 = 0;  
  int tempEntero = 0, tempDecimal;
  if(medicion >= 1000){
    unidad = "k";
    temp /= 1000;
  }else if(medicion < 1 && medicion >= 0.001){
    unidad = "m";
    temp *= 1000;
  }else if(medicion < 0.001 && medicion >= 0.000001){
    unidad = "µ";
    temp *= 1000000;
  }else if(medicion < 0.000001){
    unidad = "n";
    temp *= 1000000000;
  }else{
    unidad = "";
  }
  tempEntero = temp;
  temp2 = fmod(temp, tempEntero);
  if(tempEntero < 10){
    tempDecimal = temp2*100;
  }else if(tempEntero >= 10 && tempEntero < 100){
    tempDecimal = temp2*10;
  }else{
    tempDecimal = 0;
  }

  arr[0] = tempEntero;
  arr[1] = tempDecimal;

  parteEntera = String(salida[0]);
  parteDecimal = String(salida[1]);

}

void mostrarPantalla(char modo){
  medicion = parteEntera + "." + parteDecimal;
  switch (modo) {
  case 'r':
    pantalla.drawString(64, 10, "Medicion R");
    pantalla.drawString(55, 30, medicion);
    pantalla.drawString(85, 30, unidad + "O");
    if(unidad == ""){
      pantalla.drawString(85, 27, "_");
    }else if(unidad == "k"){
      pantalla.drawString(89, 27, "_");
    }
    break;
  case 'l':
    pantalla.drawString(64, 10, "Medicion L");
    pantalla.drawString(55, 30, medicion);
    pantalla.drawString(85, 30, unidad + "H");
    break;
  case 'c':
    pantalla.drawString(64, 10, "Medicion C");
    pantalla.drawString(55, 30, medicion);
    pantalla.drawString(85, 30, unidad + "F");
    break;
  case 'n':
    pantalla.drawString(64, 0, "NO HAY NINGUN");
    pantalla.drawString(64, 20, "MODO");
    pantalla.drawString(64, 40, "SELECCIONADO");
    break;
  default:
    pantalla.drawString(64, 0, "FUERA");
    pantalla.drawString(64, 20, "DE");
    pantalla.drawString(64, 40, "RANGO");
    break;
  }
}

void dentroFueraRango(float min, float max, float variable, char modo){
  if(variable >= min &&  variable <= max){
      numerosSalida(variable, salida);
      mostrarPantalla(modo);
    }else{
      mostrarPantalla('x'); 
    }
}

void registrarMedicion(data medicion, data* arr){
  for(int i = 199; i >= 0; i--){
    if(i != 0){
     arr[i] = arr[i-1]; 
    }else{
      arr[0] = medicion;
    }
  }
}

void verificarInactividad(data* arr){
  inactividad = 1;
  for(int y = 199; y >= 1; y--){
    if(arr[y].medicion != arr[y-1].medicion || arr[y].modo != arr[y-1].modo){
      inactividad = 0;
    }
  }
  if(inactividad == 1){
    pantalla.clear();
  }

}