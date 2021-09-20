#include <Adafruit_GFX.h>
#include <Adafruit_RA8875.h>

#define RA8875_CS 10
#define RA8875_RESET 9

#define WHITE RA8875_WHITE
#define BLACK RA8875_BLACK
#define LIGHT_GREY 0x8C71
#define DARK_GREY 0x2965
#define RED RA8875_RED
#define BLUE RA8875_BLUE

#define TEXT_SM 0
#define TEXT_MD 1
#define TEXT_LG 2
#define TEXT_XL 3

#define BYTE_TO_DEG 1.41176470588

#define LAYOUT_DIAL_RADIUS 45
#define LAYOUT_DIAL_Y 50
#define LAYOUT_ROW_LINE_Y 150
#define LAYOUT_PARAM_Y 167

typedef struct {
  bool isRendered;
  byte value;
  bool isScalar;
  bool isDisabled;
  bool isMuted;
} DialStatus;

Adafruit_RA8875 gfx_tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);

// Current state of dials used for diffing
DialStatus gfx_dials[2][CHANNEL_COUNT];

unsigned int gfx_getDialX(byte channel) {
  return (channel * 100) + 50;
}

unsigned int gfx_getDialY(byte row) {
  return row == 0 ? LAYOUT_DIAL_Y : 480 - LAYOUT_DIAL_Y;
}

void gfx_drawText(
  unsigned int x,
  unsigned int y,
  const char* buffer,
  byte size = TEXT_MD,
  unsigned int color = WHITE,
  unsigned int bgColor = BLACK,
  bool isTransp = false
) {
  gfx_tft.textMode();
  gfx_tft.textSetCursor(x, y);
  if (isTransp) {
    gfx_tft.textTransparent(color);
  } else {
    gfx_tft.textColor(color, bgColor);
  }
  gfx_tft.textEnlarge(size);
  gfx_tft.textWrite(buffer);
  gfx_tft.graphicsMode();
}

// value param is 0-255 mapped to 180-540 degrees
void gfx_drawValueLine(int xStart, int yStart, byte value, int length, int color) {
  float rads = (value * BYTE_TO_DEG + 180) * DEG_TO_RAD; // convert value to radians
  int xEnd = xStart + length * sin(rads); // Ending x-coordinate offset & radius
  int yEnd = yStart - length * cos(rads); // Ending y-coordinate offset & radius
  gfx_tft.drawLine(xStart, yStart, xEnd, yEnd, color);
}

void gfx_drawDialMarkers(unsigned int x, unsigned int y, unsigned int color) {
  unsigned int yTriB = y + LAYOUT_DIAL_RADIUS;
  unsigned int yTriT = y - LAYOUT_DIAL_RADIUS;
  gfx_tft.drawTriangle(x - 5, yTriB, x + 5, yTriB, x, yTriB - 10, color);
  gfx_tft.drawTriangle(x - 5, yTriT, x + 5, yTriT, x, yTriT + 10, color);
}

void gfx_drawDial(byte row, byte channel, byte value, const char* displayValue, bool isScalar, bool isDisabled, bool isMuted) {
  DialStatus status = gfx_dials[row][channel];
  unsigned int x = gfx_getDialX(channel);
  unsigned int y = gfx_getDialY(row);

  // Clear existing line if present
  if (status.isRendered && status.isScalar && !status.isDisabled) {
    gfx_drawValueLine(x, y, status.value, LAYOUT_DIAL_RADIUS - 10, BLACK);
  }

  // Show/hide mute indicator
  if (!status.isRendered || status.isMuted != isMuted) {
    // unsigned int yChanNo = row == 0
    //   ? y + LAYOUT_DIAL_RADIUS + 3
    //   : y - LAYOUT_DIAL_RADIUS - 35;
    // gfx_drawText(x + 15, yChanNo, "M", TEXT_MD, BLACK, isMuted ? BLUE : BLACK);
    gfx_drawDialMarkers(x, y, isMuted ? LIGHT_GREY : RED);
  }

  // Print display value
  int textColor = (isDisabled || isMuted) ? LIGHT_GREY : WHITE;
  gfx_drawText(x - 25, y - 18, displayValue, TEXT_MD, textColor);

  if (isScalar && !isDisabled) {
    // Draw value line
    gfx_drawValueLine(x, y, value, LAYOUT_DIAL_RADIUS - 10, isMuted ? LIGHT_GREY : RED);
  }

  gfx_dials[row][channel] = {
    .isRendered = true,
    .value = value,
    .isScalar = isScalar,
    .isDisabled = isDisabled,
    .isMuted = isMuted
  };
}

void gfx_drawFlash(const char* msg) {
  char buffer[31];
  sprintf(buffer, "%30s", msg);
  gfx_drawText(300, LAYOUT_PARAM_Y + 3, buffer);
}

void gfx_clearFlash(void) {
  gfx_drawFlash("                              "); // 30 spaces
}

void gfx_drawParamName(byte row, const char* name) {
  // Right-pad name to fully overwrite previous name
  char buffer[11];
  sprintf(buffer, "%-10s", name);
  gfx_drawText(8, row == 0 ? LAYOUT_PARAM_Y : 426 - LAYOUT_PARAM_Y, buffer, TEXT_LG, LIGHT_GREY);
}

void gfx_drawStaticElements(void) {
  unsigned int x;
  unsigned int y;
  unsigned int yChanNo;
  char buffer [2];

  // Draw horizontal dividers
  gfx_tft.drawFastHLine(0, LAYOUT_ROW_LINE_Y, 800, LIGHT_GREY);
  gfx_tft.drawFastHLine(0, 480 - LAYOUT_ROW_LINE_Y, 800, LIGHT_GREY);

  // Draw row static elements
  for (byte row = 0; row < 2; row++) {
    y = gfx_getDialY(row);
    yChanNo = row == 0 ? y + LAYOUT_DIAL_RADIUS + 3 : y - LAYOUT_DIAL_RADIUS - 35;
    for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
      x = gfx_getDialX(channel);

      // Draw channel number
      itoa(channel + 1, buffer, 10);
      gfx_drawText(x - 8, yChanNo, buffer, TEXT_MD, LIGHT_GREY);
    }
  }
}

void gfx_start(void) {
  gfx_tft.fillScreen(BLACK);
  // for (byte b = 0; b < CHANNEL_COUNT; b++) {
  //   gfx_dials[0][b] = {};
  //   gfx_dials[1][b] = {};
  // }
  gfx_drawStaticElements();
}

void gfx_setup(void) {
  // Start TFT
  bool ok = gfx_tft.begin(RA8875_800x480);
  if (!ok) {
    Serial.println("RA8875 Not Found!");
    return;
  }
  gfx_tft.displayOn(true);
  gfx_tft.GPIOX(true); // Enable TFT - display enable tied to GPIOX
  gfx_tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  gfx_tft.PWM1out(255);

  gfx_drawText(100, 260, "Initializing...", TEXT_LG, WHITE);
}

void __gfx_log(const char* s) {
  gfx_drawText(500, 240, s);
}