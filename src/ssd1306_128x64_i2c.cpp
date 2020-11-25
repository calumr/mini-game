#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  for (int pin = 2; pin < 9; ++pin)
  {
    pinMode(pin, INPUT_PULLUP);
  }
}

const int BlockSize = 2;
const int VBlocks = SCREEN_HEIGHT / BlockSize;
const int HBlocks = 16;

using Row = uint16_t;
using Screen = Row[VBlocks];
using Piece = Row[4];

Screen screen = {0};
Piece current = {0};
int pieceY = VBlocks;

bool willLand()
{
  for (int y = 0; y < 4; ++y)
  {
    int screenY = y + pieceY;

    if (screenY <= 0)
    {
      if (current[y] != 0)
      {
        Serial.println(F("below 0"));
        Serial.println(pieceY);
        Serial.println(screenY);
        Serial.println(y);
        return true;
      }
    }
    else if (screenY < VBlocks)
    {
      if (screen[screenY - 1] & current[y])
      {
        Serial.println(F("hit screen"));
        Serial.println(pieceY);
        Serial.println(screenY);
        Serial.println(y);
        return true;
      }
    }
  }

  return false;
}

void landPiece()
{
  if (pieceY >= VBlocks - 1)
  {
    display.setCursor(32, 40);
    display.println(F("Game Over!"));
    display.display();

     while (true){delay(10000);}
  }

  for (int y = 0; y < 4; ++y)
  {
    int screenY = y + pieceY;

    screen[screenY] |= current[y];
  }

  pieceY = VBlocks;
}

void tryMoveLeft()
{
  for (int y = 0; y < 4; ++y)
  {
    if (current[y] & 0b1000000000000000)
      return;
  }

  for (int y = 0; y < 4; ++y)
  {
    current[y] <<= 1;
  }
}

void tryMoveRight()
{
  for (int y = 0; y < 4; ++y)
  {
    if (current[y] & 0b0000000000000001)
      return;
  }

  for (int y = 0; y < 4; ++y)
  {
    current[y] >>= 1;
  }
}

void loop()
{
  if (pieceY == VBlocks)
  {
    current[0] = 0b0001000000000000;
    current[1] = 0b0001000000000000;
    current[2] = 0b0001100000000000;
    current[3] = 0b0000000000000000;
  }

  display.clearDisplay();
  for (int y = 0; y < VBlocks; ++y)
  {
    for (int x = 0; x < HBlocks; ++x)
    {
      const Row mask = 1 << x;
      bool isBlockOn = screen[y] & mask;
      if (y >= pieceY && y < pieceY + 4)
      {
        isBlockOn |= current[y - pieceY] & mask;
      }

      if (isBlockOn){
        display.fillRect(x * BlockSize, SCREEN_HEIGHT - (y + 1) * BlockSize, BlockSize, BlockSize, SSD1306_WHITE);
      }
    }
  }

  --pieceY;

  const bool set = digitalRead(2) == HIGH;
  const bool reset = digitalRead(3) == HIGH;

  const bool right = digitalRead(5) == HIGH;
  const bool left = digitalRead(6) == HIGH;
  const bool down = digitalRead(7) == HIGH;
  const bool up = digitalRead(8) == HIGH;

  if (willLand())
  {
    landPiece();
  }
  else
  {
    if (left)
      tryMoveLeft();
    if (right)
      tryMoveRight();

    if (willLand())
    {
      landPiece();
    }
  }

  display.display();

  delay(200);
}
