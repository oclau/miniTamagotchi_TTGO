#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS    5
#define TFT_DC   16
#define TFT_RST  23
#define TFT_BL    4

#define JOY_X 39
#define JOY_Y 32
#define JOY_SW 33

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Stany
int hunger = 70;
int sleepiness = 40;
int mood = 85;
bool isSick = false;

int selected = 0; // 0-hunger, 1-sleep, 2-mood
unsigned long lastJoyTime = 0;

void drawCatFace() {
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(40, 10);  tft.println("  /\\_/\\  ");
  tft.setCursor(40, 30);  tft.println(" ( o.o ) ");
  tft.setCursor(40, 50);  tft.println("  > ^ < ");
}

void drawStaticScreen() {
  tft.fillScreen(ST77XX_BLACK);
  drawCatFace();

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(10, 90);  tft.print("Hunger:");
  tft.setCursor(10, 105); tft.print("Sleep:");
  tft.setCursor(10, 120); tft.print("Mood:");
}

void updateBars() {
  for (int i = 0; i < 3; i++) {
    int val = (i == 0) ? hunger : (i == 1) ? sleepiness : mood;
    int y = 90 + i * 15;

    tft.fillRect(70, y, 80, 8, ST77XX_BLACK);
    tft.fillRect(70, y, val * 0.6, 8, (i == 0) ? ST77XX_RED : (i == 1) ? ST77XX_BLUE : ST77XX_GREEN);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(160, y);
    tft.printf("%3d%%", val);
  }

  drawSelector();
}

void drawSelector() {
  for (int i = 0; i < 3; i++) {
    tft.drawRect(5, 89 + i * 15, 150, 10, ST77XX_BLACK);
  }
  if (!isSick) {
    tft.drawRect(5, 89 + selected * 15, 150, 10, ST77XX_WHITE);
  }
}

void showSickMessage() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(20, 40);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.println("Kot cierpi!");
  tft.setCursor(20, 80);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Wcisnij guzik by uleczyc");
}

void handleJoystick() {
  int y = analogRead(JOY_Y);
  int btn = digitalRead(JOY_SW);
  unsigned long now = millis();

  if (now - lastJoyTime > 300 && !isSick) {
    if (y < 1000 && selected > 0) {
      selected--;
      lastJoyTime = now;
    } else if (y > 3000 && selected < 2) {
      selected++;
      lastJoyTime = now;
    }
  }

  if (btn == LOW) {
    if (isSick) {
      hunger = 50;
      sleepiness = 50;
      mood = 50;
      isSick = false;
      drawStaticScreen();
      updateBars();
    } else {
      switch (selected) {
        case 0: hunger = min(100, hunger + 10); break;
        case 1: sleepiness = max(0, sleepiness - 10); break;
        case 2: mood = min(100, mood + 10); break;
      }
    }
    delay(300); // debounce
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  pinMode(JOY_SW, INPUT_PULLUP);

  tft.init(135, 240);
  tft.setRotation(1);
  drawStaticScreen();
  updateBars();
}

void loop() {
  handleJoystick();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000 && !isSick) {
    hunger = max(0, hunger - 1);
    sleepiness = min(100, sleepiness + 1);
    mood = max(0, mood - 1);
    lastUpdate = millis();
  }

  if ((hunger == 0 || sleepiness == 100 || mood == 0) && !isSick) {
    isSick = true;
    showSickMessage();
  }

  if (!isSick) updateBars();
  delay(100);
}
