
// Wir müssen diese Bibliotheken mittels Bibliotheksmanager installieren
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>

// Anzahl der "NeoPixel"
#define NUM_PIXELS 12

#define CENTER_LED 6

// Mit welchem ​​GPIO ist der LED-Strip verbunden?
#define PIXEL_PIN 33

#define COLOR_ORDER RGB

// Bereich 0-64
#define BRIGHTNESS 50

const char* SSID = "PixelChaser";
const char* PASSWORD = "pw";

// Unser Webserver, welcher das Interface bereit stellt
AsyncWebServer server(80);

// Definition der Schwierigkeitsgrade
#define EASY 1
#define MEDIUM 2
#define HARD 3
#define ON_SPEED 4
#define SONIC_SPEED 5
#define ROCKET_SPEED 6
#define LIGHT_SPEED 7
#define MISSION_IMPOSSIBLE 8

// Aktueller Schwierigkeitsgrad
int difficulty = EASY;

// Definition des Pixelarray
CRGB pixels[NUM_PIXELS];

// Hat der Spieler diese Runde gewonnen? Dieses Tag wird für Schwierigkeitsparameter verwendet.
bool wonThisRound = false;

// Start Position des Cycling-Effekt
int pixelAddress = 0;

// Läuft das Spiel?
bool playing = false;

// Help-Variablen für langen Button-Press
long buttonTimer = 0;
long longPressTime = 300;

void setup()
{
  // Serielle Schnittstelle mit 115200 Bit pro Sekunde Datenrate initialisieren
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
    Serial.println("Dateisystem konnte nicht initialisiert weren.");
    return;
  }

  // Den LED-Streifen initialisieren und die Helligkeit einstellen
  FastLED.addLeds<NEOPIXEL, PIXEL_PIN>(pixels, NUM_PIXELS); // Init der Fastled-Bibliothek
  FastLED.setBrightness(BRIGHTNESS);

  // WiFi Accesspoint starten und die IP-Adresse ausgeben
  // Die IP-Adresse ist gleichzeitig die des Webservers!
  WiFi.softAP(SSID, PASSWORD);
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.softAPIP());

  // CSS Datei bereitstellen
  server.on("/main.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("incomming request, serving main.css");
    request->send(SPIFFS, "/main.css");
  });

  // JavaScript Datei bereitstellen
  server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("incomming request, serving main.js");
    request->send(SPIFFS, "/main.js");
  });

  // HTML Datei bereitstellen
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("incomming request, serving index.html");
    request->send(SPIFFS, "/index.html");
  });

  // Button press-API
  server.on("/button-press", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "");
    Serial.println("button was pressed");
    buttonTimer = millis();
  });

  // Button release-API
  server.on("/button-release", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "");
    Serial.println("button was released");
    if (buttonWasLongPressed()) {
      if (playing)
      {
        turnOff();
      }
      else {
        turnOn();
      }
    }
    else {
      checkLedPosition();
    }
  });

  server.begin();

  delay(1000);

  startUp();
}

void loop()
{
  // PLAYING
  if (playing)
  {
    drawFrame();
    pixelAddress++; // Setzt die aktuelle LED immer eine weiter
    if (pixelAddress == NUM_PIXELS)
    {
      // Wenn wir am Ende angelangt sind, beginnen wir wieder von vorne (0)
      pixelAddress = 0;
    }
    delay(getTime(difficulty));
  }
}

void startUp() {
  playing = false;
  for (int i = 0; i < NUM_PIXELS; i++)
  {
    pixels[i] = CRGB::Green;
  }
  FastLED.show();
  delay(1000);
  turnOff();
}

void turnOff() {
  playing = false;
  for (int i = 0; i < NUM_PIXELS; i++)
  {
    pixels[i] = CRGB::Black; // Schaltet alle LEDs aus
  }
  FastLED.show();
}

void turnOn() {
  difficulty = EASY;
  cylon(8);
  playing = true;
}

bool buttonWasLongPressed() {
  return millis() - buttonTimer > longPressTime;
}

void checkLedPosition() {
  if (playing)
  {
    playing = false; // Der Benutzer hat die Taste gedrückt und die LED ist angehalten
    drawFrame();

    int diff = abs(CENTER_LED - pixelAddress); // Findet den Abstand zwischen der beleuchteten LED und der mittleren LED
    if (diff == 0)
    {
      wonThisRound = true; // Der Spieler hat das Level erfolgreich geschlagen
      if (difficulty != MISSION_IMPOSSIBLE)
      {
        cylon(8);
      }
      if (difficulty == MISSION_IMPOSSIBLE)
      {
        cylon(20);
        difficulty = 0;
      }
      increaseDifficulty();
      wonThisRound = false;
    }
    else
    {
      delay(1000);
      flash();
    }
    pixelAddress = 0;
    delay(250);
    playing = true;
  }
}

void drawFrame() {
  for (int i = 0; i < NUM_PIXELS; i++)
  {
    pixels[i] = CRGB::Black; // Schaltet alle LEDs aus
  }
  pixels[CENTER_LED] = CRGB::Red; // Setzt die Farbe der mittleren LED auf Rot
  pixels[pixelAddress] = CRGB::Blue; // Setzt die Farbe der Cyling-LED auf Blau
  FastLED.show(); // Initialisiert den Lichtzyklus
}

// Level Parameter
int getTime(int diff) // Gibt die Zeitverzögerung für die LED-Bewegung basierend auf dem Schwierigkeitsgrad zurück
{
  int timeValue = 0;
  switch (diff)
  {
    case EASY:
      timeValue = 200;
      break;
    case MEDIUM:
      timeValue = 130;
      break;
    case HARD:
      timeValue = 90;
      break;
    case ON_SPEED:
      timeValue = 60;
      break;
    case SONIC_SPEED:
      timeValue = 40;
      break;
    case ROCKET_SPEED:
      timeValue = 30;
      break;
    case LIGHT_SPEED:
      timeValue = 20;
      break;
    case MISSION_IMPOSSIBLE:
      timeValue = 10;
  }
  return timeValue; // Gibt den Verzögerungsbetrag zurück
}

// Gewinnschwierigkeite erhöhen
void increaseDifficulty()
{
  if (difficulty != MISSION_IMPOSSIBLE && wonThisRound)
  {
    difficulty++;
  }
}

// LED-Show "Verloren"
void flash()
{
  for (int i = 0; i < 3; i++)
  {
    fill_solid(pixels, NUM_PIXELS, CRGB::Red);
    FastLED.show();
    delay(500);
    fill_solid(pixels, NUM_PIXELS, CRGB::Black);
    FastLED.show();
    delay(500);
  }
}

// LED-Show "Gewonnen"
void fadeall()
{
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels[i].nscale8(250);
  }
}

// Farb-Regenbogen-Effekt
void cylon(int amount)
{
  for (int i = 0; i < amount; i++)
  {
    static uint8_t hue = 0;
    Serial.print("x");
    for (int i = 0; i < NUM_PIXELS; i++) {
      pixels[i] = CHSV(hue++, 255, 255);
      FastLED.show();
      fadeall();
      delay(10);
    }
    Serial.print("x");
    for (int i = (NUM_PIXELS) - 1; i >= 0; i--) {
      pixels[i] = CHSV(hue++, 255, 255);
      FastLED.show();
      fadeall();
      delay(10);
    }
  }
}
