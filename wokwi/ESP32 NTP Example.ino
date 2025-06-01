// Learn about the ESP32 WiFi simulation in
// https://docs.wokwi.com/guides/esp32-wifi
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

//módulos para webservice
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string>

LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 16, 2);

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     0
#define UTC_OFFSET_DST 0




#define dhtPino 14 // Pino do sensor DHT22
#define DHT_TIPO DHT22

DHT dht(dhtPino, DHT22); 
Adafruit_MPU6050 mpu;


// --- CONFIGURAÇÕES DE CALIBRAÇÃO E MONITORAMENTO ---
const int AMOSTRAS_CALIBRACAO = 30; // Número de leituras para estabelecer a baseline
float baselineVibracaoMedia = 0.0;   // Armazena a vibração média normal
float limiteAlerta = 0.0;            // Limite para um alerta de vibração (ex: 1.5x a baseline)
float limitePerigo = 0.0;            // Limite para um alerta de perigo (ex: 2.5x a baseline)

// Variáveis de controle de estado
int contadorAmostras = 0;
bool calibrando = true;

//método http para passar dados dos sensores no web-service
HTTPClient http; 

void spinner() {
  static int8_t counter = 0;
  const char* glyphs = "\xa1\xa5\xdb";
  LCD.setCursor(15, 1);
  LCD.print(glyphs[counter++]);
  if (counter == strlen(glyphs)) {
    counter = 0;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  LCD.init();
  LCD.backlight();
  LCD.setCursor(0, 0);
  LCD.print("Conectando ao ");
  LCD.setCursor(0, 1);
  LCD.print("WiFi ");

  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    spinner();
  }

  dht.begin();

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.println("Online");
  LCD.setCursor(0, 1);
  LCD.println("Atualizar Hora");

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  
  Wire.begin(21, 22);
  

  while (!mpu.begin()) {
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.println("Falha  MPU6050");
    delay(1000);  // Aguarda 1 segundo antes de tentar novamente
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.println("Iniciando");
  LCD.setCursor(0, 1);
  LCD.print("Calibracao");
}

void loop() {
  delay(2050);
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  // Verifica se houve erro na leitura do DHT22
  if (isnan(temperatura) || isnan(umidade)) {
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.println("Falha  DHT22");
    return;
  }
  

  acelerometroFuncionamento(temperatura, umidade);

}

void acelerometroFuncionamento(float temperatura, float umidade){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  if(temperatura > 40 || temperatura < -20){
    LCD.clear();
    LCD.setCursor(0, 0);
    if(temperatura > 40)
      LCD.println("Temperatura Alta");
    else
      LCD.println("Temperatura Baixa");
    LCD.setCursor(0,1);
    LCD.println("Desligue equipamento");
    //callWs("PERIGO", temperatura, umidade, 0.0);
    return;
  }

  if(umidade > 95 || umidade < 15){
    LCD.clear();
    LCD.setCursor(0, 0);
    if(umidade > 95)
      LCD.println("Umidade Alta");
    else
      LCD.println("Umidade Baixa");
    LCD.setCursor(0,1);
    LCD.println("Desligue equipamento");
    //callWs("PERIGO", temperatura, umidade, 0.0);
    return;
  }

  char* status = "";
  float x = a.acceleration.x;
  float y = a.acceleration.y;
  float z = a.acceleration.z;
  float vibracaoAtual = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

  if (calibrando) {
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.println("Calibrando... ");
    LCD.setCursor(0, 1);
    LCD.print("[Amostra ");
    LCD.print(contadorAmostras + 1);
    LCD.print("/");
    LCD.print(AMOSTRAS_CALIBRACAO);
    LCD.print("] -");
    LCD.print(vibracaoAtual);

    baselineVibracaoMedia += vibracaoAtual;
    contadorAmostras++;

    if (contadorAmostras >= AMOSTRAS_CALIBRACAO) {
      baselineVibracaoMedia = baselineVibracaoMedia / AMOSTRAS_CALIBRACAO;
      
      limiteAlerta = baselineVibracaoMedia * 1.5; 
      limitePerigo = baselineVibracaoMedia * 2.5;

      calibrando = false;
      LCD.clear();
      LCD.setCursor(0, 0);
      LCD.println("Calibracao");
      LCD.setCursor(0, 1);
      LCD.println("concluida");
    }
    
  } else {
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print("Vibracao: ");
    LCD.print(vibracaoAtual);
    LCD.setCursor(0, 1);

    if (vibracaoAtual > limitePerigo) {
      status = "PERIGO";
      LCD.print("PERIGO! DESLIGAR");
    } else if (vibracaoAtual > limiteAlerta) {
      status = "ALERTA";
      LCD.print("Status: Alerta!");
    } else {
      status = "NORMAL";
      LCD.print("Status: Normal");
    }
  }

  //habilitar quando for capturar os dados
  // if(!calibrando){
    // callWs(status, temperatura, umidade, vibracaoAtual);
  // }
  Serial.print(" temperatura: ");
  Serial.print(temperatura);
  Serial.print(" umidade: ");
  Serial.print(umidade);
  Serial.print(" vibracao: ");
  Serial.println(vibracaoAtual);
  delay(calibrando ? 200 : 1000); 
}


void callWs(char* status, float temperatura, float umidade, float vibracaoAtual ){
  //link do webservice
  http.begin("https://newsfacd.herokuapp.com/fiap/challenge"); 
  http.addHeader("Content-Type", "application/json");
  
  //formar arquivo json
  StaticJsonDocument<1024> doc;
  doc["status"] = status;
  doc["temperatura"] = temperatura;
  doc["umidade"] = umidade;
  doc["vibracao"] = vibracaoAtual;

  String httpRequestData;
  serializeJson(doc, httpRequestData);

  Serial.println("Enviando JSON para o WebService:");
  Serial.println(httpRequestData);

  int httpResponseCode = http.POST(httpRequestData);
  if (httpResponseCode > 0) {
    Serial.printf("Código de Resposta HTTP: %d\n", httpResponseCode);
    String payload = http.getString();
    Serial.println("Resposta: " + payload);
  } else {
    Serial.printf("Falha na requisição HTTP, erro: %s\n", http.errorToString(httpResponseCode).c_str());
  }
}