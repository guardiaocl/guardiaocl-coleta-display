#include <dht.h>
#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define DHTPIN 7 //porta conectada ao DHT11
static char apiKey[] = "xxxxxxxx-yyyy-zzzz-wwww-kkkkkkkkkkkk"; //@todo Alterar para a API Key do Projeto.
static char serial[] = "GUC0001"; //@todo Alterar para a Serial do Dispositivo.

dht DHT;
EthernetClient client;
LiquidCrystal_I2C lcd(0x27,16,2);

signed long next;
unsigned long frequencyTime = 60000; //Frequência do Tempo que será lido e enviado os valores(1 min.).
static byte mac[] = { 0xDE, 0xAD, 0x09, 0xEF, 0xFE, 0xED }; // Endereço MAC, mude caso tenha vários
static char serverName[] = "guardiao.cl"; //NÃO ALTERE ESSE VALOR
static byte termometru[8] = //icone para temperatura
{
  B00100,
  B01010,
  B01010,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110
};

static byte picatura[8] = //icone de humidade
{
  B00100,
  B00100,
  B01010,
  B01010,
  B10001,
  B10001,
  B10001,
  B01110,
};

void setup() {
  lcd.init();
  lcd.clear();
  lcd.backlight(); 
  lcd.print(F("System Starting"));
  lcd.setCursor(0, 1);
  lcd.print(F("Guardiao Cloud"));
  lcd.createChar(1,termometru);
  lcd.createChar(2,picatura);

  Ethernet.begin(mac); //IP Automático
  next = 0;
}

void loop() {
  if (((signed long)(millis() - next)) > 0)
  {
    next = millis() + frequencyTime;
    char temp[6] PROGMEM;
    char hum[6] PROGMEM;
    float temperature = 0;
    float humidity = 0;
    int chk = DHT.read11(DHTPIN); //Leitura do Sensor DHT11
    if(chk == DHTLIB_OK){  
      temperature = DHT.temperature;
      humidity = DHT.humidity;
      dtostrf(temperature, 1, 2, temp);
      dtostrf(humidity, 1, 2, hum);
    }
    else{
      temp[0] = '0';
      hum[0] = '0';
    }
    char queryString[128];
    char outBuf[128];
    char host[64];

    lcd.clear();
    lcd.setCursor(0, 0);  
    lcd.print(F("Serie - "));
    lcd.print(serial);
    lcd.setCursor(1, 1);  
    lcd.write(1);
    lcd.setCursor(3, 1);
    lcd.print((float)temperature,0);
    lcd.setCursor(5, 1);
    lcd.print((char)223); //sinal de graus
    lcd.print(F("C"));

    lcd.setCursor(9, 1);
    lcd.write(2);
    lcd.setCursor(11, 1);
    lcd.print((float)humidity,0);
    lcd.print(F("%"));

    if (client.connect(serverName, 80)) 
    {
      sprintf(queryString,"apiKey=%s&temperatura=%s&humidade=%s",apiKey, temp,hum);
      sprintf(outBuf,"GET /collect/%s/?%s HTTP/1.1",serial,queryString);
      client.println(outBuf);
      sprintf(host,"Host: %s",serverName);
      client.println(host);
      client.println("User-Agent: arduino-ethernet");
      client.println("Connection: close");
      client.println();
      client.stop();
    }
    else
    {
      Serial.println(F("Falha de conexão"));
    } 
  }

}