#include <UbiConstants.h>
#include <UbiTypes.h>
#include <UbidotsEsp32Mqtt.h>
#include "DHT.h"
#include <TFT_eSPI.h>
#include <SPI.h>

/****************************************
 * Define Constants
 ****************************************/
#define DHTPIN 12            // Pin conectado al sensor DHT11
#define DHTTYPE DHT11        // Tipo de sensor DHT11
const char *UBIDOTS_TOKEN = "BBUS-UmLWaIxEkCI6nYJel8pIL3zd96IPJa";  // Token de Ubidots
const char *WIFI_SSID = "Lasso";      // Nombre de tu red Wi-Fi
const char *WIFI_PASS = "24825207";   // Contraseña de tu red Wi-Fi
const char *DEVICE_LABEL = "esp32";   // Nombre del dispositivo en Ubidots
const char *TEMP_LABEL = "temp";      // Etiqueta para la temperatura
const char *HUMIDITY_LABEL = "humidity"; // Etiqueta para la humedad

const int PUBLISH_FREQUENCY = 5000;   // Frecuencia de envío en milisegundos

unsigned long timer;

// Inicializa el sensor DHT
DHT dht(DHTPIN, DHTTYPE);

// Inicializa el cliente Ubidots
Ubidots ubidots(UBIDOTS_TOKEN);

// Inicializa la pantalla OLED
TFT_eSPI tft = TFT_eSPI(135, 240); // Pantalla de la TTGO OLED

/****************************************
 * Funciones Auxiliares
 ****************************************/
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void initDisplay()
{
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Inicializando...", tft.width() / 2, tft.height() / 2);
  delay(2000);
  tft.fillScreen(TFT_BLACK);
}

/****************************************
 * Funciones principales
 ****************************************/

void setup()
{
  // Configuración inicial
  Serial.begin(115200);
  dht.begin(); // Inicia el sensor DHT
  
  // Inicializa la pantalla OLED
  initDisplay();
  
  // Conexión Wi-Fi y Ubidots
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  timer = millis();
}

void displayData(float temperature, float humidity)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 20);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  tft.printf("Temp: %.2f C\n", temperature);
  tft.printf("Hum: %.2f %%\n", humidity);

  Serial.printf("Temp: %.2f C, Hum: %.2f %%\n", temperature, humidity);
}

void loop()
{
  // Reconexión a Ubidots si es necesario
  if (!ubidots.connected())
  {
    ubidots.reconnect();
  }

  // Enviar datos periódicamente
  if (abs((long)(millis() - timer)) > PUBLISH_FREQUENCY)
  {
    float temperature = dht.readTemperature(); // Lectura de temperatura
    float humidity = dht.readHumidity();      // Lectura de humedad

    // Verifica si las lecturas son válidas
    if (isnan(temperature) || isnan(humidity))
    {
      Serial.println("Error al leer el sensor DHT11");
      tft.fillScreen(TFT_RED);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("Error sensor!", tft.width() / 2, tft.height() / 2);
    }
    else
    {
      // Muestra los datos en la pantalla OLED
      displayData(temperature, humidity);

      // Enviar datos a Ubidots
      ubidots.add(TEMP_LABEL, temperature);   // Añadir temperatura
      ubidots.add(HUMIDITY_LABEL, humidity); // Añadir humedad
      ubidots.publish(DEVICE_LABEL);         // Publicar al dispositivo
    }
    timer = millis(); // Reinicia el temporizador
  }

  ubidots.loop();
}
