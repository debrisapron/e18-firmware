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
#define YELLOW RA8875_YELLOW

#define TEXT_SM 0
#define TEXT_MD 1
#define TEXT_LG 2
#define TEXT_XL 3

#define BYTE_TO_DEG 1.41176470588

#define DIAL_ROW_HEIGHT 150
#define DIAL_ROW_MARGIN 17
#define OLD_DIAL_RADIUS 48
#define DIAL_RADIUS 38
#define DIAL_MARKER_HEIGHT 10
#define DIAL_MARKER_WIDTH DIAL_MARKER_HEIGHT
#define DIAL_MARKER_HALF_WIDTH DIAL_MARKER_WIDTH / 2
#define DIAL_CENTER_Y 50
#define TEXT_MD_LINE_HEIGHT 32 // From y coord to next line y coord
#define TEXT_LG_HEIGHT 54 // From y coord to bottom of character
#define MENU_ITEM_LEFT_PADDING 12
#define MENU_ITEM_ROW_PADDING 12 // Distance from row hline
#define PARAM_LABEL_LEFT_MARGIN 8
#define PARAM_LABEL_TOP DIAL_ROW_HEIGHT + DIAL_ROW_MARGIN
#define PARAM_LABEL_BOTTOM 480 - (PARAM_LABEL_TOP + TEXT_LG_HEIGHT)

typedef struct {
  bool isRendered;
  byte value;
  byte chaState;
  bool isScalar;
  bool isBipolar;
  bool isDisabled;
  bool isSilent;
} DialStatus;

Adafruit_RA8875 gfx_tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);

// Current state of dials used for diffing
DialStatus gfx_dials[2][CHANNEL_COUNT];

unsigned int gfx_getDialX(byte channel) {
  return (channel * 100) + 50;
}

unsigned int gfx_getDialY(byte row) {
  return row == 0 ? DIAL_CENTER_Y : 480 - DIAL_CENTER_Y;
}

void gfx_drawText(
  unsigned int x,
  unsigned int y,
  const char *buffer,
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
  unsigned int yTriB = y + (DIAL_RADIUS + DIAL_MARKER_HEIGHT);
  unsigned int yTriT = y - (DIAL_RADIUS + DIAL_MARKER_HEIGHT);
  gfx_tft.drawTriangle(x - DIAL_MARKER_HALF_WIDTH, yTriB, x + DIAL_MARKER_HALF_WIDTH, yTriB, x, yTriB - DIAL_MARKER_HEIGHT, color);
  gfx_tft.drawTriangle(x - DIAL_MARKER_HALF_WIDTH, yTriT, x + DIAL_MARKER_HALF_WIDTH, yTriT, x, yTriT + DIAL_MARKER_HEIGHT, color);
}

void gfx_drawValueSegment(unsigned int x, unsigned int y, byte value, unsigned int radius, unsigned int color) {
  float rads1 = (value * BYTE_TO_DEG + 180) * DEG_TO_RAD; // convert value to radians
  int x1 = x + radius * sin(rads1);
  int y1 = y - radius * cos(rads1);
  float rads2 = ((value + 2) * BYTE_TO_DEG + 180) * DEG_TO_RAD; // convert value to radians
  int x2 = x + radius * sin(rads2);
  int y2 = y - radius * cos(rads2);
  gfx_tft.drawLine(x1, y1, x2, y2, color);
}

bool gfx_segStatus(byte segmentIndex, byte value, bool isBipolar) {
  if (!isBipolar || segmentIndex > 127) return segmentIndex < value;
  return segmentIndex >= value;
}

void gfx_drawChanName(byte row, byte chan, const char *name, unsigned int color, unsigned int bgColor) {
  unsigned int top = row
    ? (480 - DIAL_ROW_HEIGHT) + MENU_ITEM_ROW_PADDING
    : DIAL_ROW_HEIGHT - (MENU_ITEM_ROW_PADDING + TEXT_MD_LINE_HEIGHT);
  unsigned int left = chan * 100 + MENU_ITEM_LEFT_PADDING;
  gfx_drawText(left, top, name, TEXT_MD, color, bgColor);
}

void gfx_drawDial(
  byte row,
  byte chan,
  byte value,
  const char *chanName,
  const char *displayValue,
  byte chaState,
  bool isScalar,
  bool isBipolar,
  bool isDisabled,
  bool isSilent
) {
  DialStatus status = gfx_dials[row][chan];
  unsigned int x = gfx_getDialX(chan);
  unsigned int y = gfx_getDialY(row);

  // Draw channel name
  if (!status.isRendered || status.chaState != chaState) {
    switch (chaState) {
      case CHA_STATE_MUTED:
        gfx_drawChanName(row, chan, chanName, BLACK, BLUE);
        break;
      case CHA_STATE_SOLOED:
        gfx_drawChanName(row, chan, chanName, BLACK, YELLOW);
        break;
      default:
        gfx_drawChanName(row, chan, chanName, LIGHT_GREY, BLACK);
        break;
    }
  }

  // Draw dial markers
  if (!status.isRendered || status.isSilent != isSilent) {
    gfx_drawDialMarkers(x, y, isSilent ? LIGHT_GREY : RED);
  }

  // Print display value
  int textColor = (isDisabled || isSilent) ? LIGHT_GREY : WHITE;
  gfx_drawText(x - 33, y - 18, displayValue, TEXT_MD, textColor);

  bool isRingRendered = status.isRendered && status.isScalar && !status.isDisabled;
  bool ringShouldBeRendered = isScalar && !isDisabled;

  // Clear existing pointer if present
  if (isRingRendered) {
    gfx_drawValueLine(x, y, status.value, DIAL_RADIUS, BLACK);
  }

  // Draw new pointer if required
  if (ringShouldBeRendered) {
    gfx_drawValueLine(x, y, value, DIAL_RADIUS, isSilent ? LIGHT_GREY : RED);
  }

  // Update circle segment by segment
  for (byte seg = 0; seg < 254; seg = seg + 2) {
    bool isOn = isRingRendered && gfx_segStatus(seg, status.value, status.isBipolar);
    bool shouldBeOn = ringShouldBeRendered && gfx_segStatus(seg, value, isBipolar);
    if (isOn != shouldBeOn) {
      unsigned int color = shouldBeOn
        ? (isSilent ? LIGHT_GREY : RED)
        : BLACK;
      gfx_drawValueSegment(x, y, seg, DIAL_RADIUS, color);
    }
  }

  gfx_dials[row][chan] = {
    .isRendered = true,
    .value = value,
    .chaState = chaState,
    .isScalar = isScalar,
    .isBipolar = isBipolar,
    .isDisabled = isDisabled,
    .isSilent = isSilent
  };
}

bool gfx_isLabelBlank(const char text[6]) {
  for (byte i = 0; i < 5; i++) {
    if (text[i] != ' ') return false;
  }
  return true;
}

// isEditable defaults to false
void gfx_drawMenuItem(byte row, byte chan, const char *text[3], bool isEditable) {
  unsigned int color = isEditable ? BLACK : WHITE;
  unsigned int bgColor = isEditable ? WHITE: BLACK;
  unsigned int top = row
    ? (480 - DIAL_ROW_HEIGHT) + MENU_ITEM_ROW_PADDING
    : DIAL_ROW_HEIGHT - (MENU_ITEM_ROW_PADDING + TEXT_MD_LINE_HEIGHT * 3);
  unsigned int left = chan * 100 + MENU_ITEM_LEFT_PADDING;
  // TODO: only draw text if line is not blank
  if (!gfx_isLabelBlank(text[0]))
    gfx_drawText(left, top, text[0], TEXT_MD, color, bgColor);
  if (!gfx_isLabelBlank(text[1]))
    gfx_drawText(left, top + TEXT_MD_LINE_HEIGHT, text[1], TEXT_MD, color, bgColor);
  if (!gfx_isLabelBlank(text[2]))
    gfx_drawText(left, top + TEXT_MD_LINE_HEIGHT * 2, text[2], TEXT_MD, color, bgColor);
}

void gfx_drawFlash(const char *msg) {
  char buffer[31];
  sprintf(buffer, "%30s", msg);
  gfx_drawText(300, PARAM_LABEL_TOP + 3, buffer);
}

void gfx_clearFlash(void) {
  gfx_drawFlash("                              "); // 30 spaces
}

void gfx_drawParamName(byte row, const char *name) {
  // Right-pad name to fully overwrite previous name
  char buffer[11];
  sprintf(buffer, "%-10s", name);
  gfx_drawText(PARAM_LABEL_LEFT_MARGIN, row == 0 ? PARAM_LABEL_TOP : PARAM_LABEL_BOTTOM, buffer, TEXT_LG, LIGHT_GREY);
}

void gfx_clear(void) {
  unsigned int x;
  for (byte chan = 0; chan < 8; chan++) {
    x = (chan * 100) + 1;
    gfx_tft.fillRect(x, 1, 98, DIAL_ROW_HEIGHT - 2, BLACK);
    gfx_tft.fillRect(x, 480 - (DIAL_ROW_HEIGHT - 1), 98, DIAL_ROW_HEIGHT - 2, BLACK);
    gfx_dials[0][chan].isRendered = false;
    gfx_dials[1][chan].isRendered = false;
  }
  gfx_drawParamName(0, "");
  gfx_drawParamName(1, "");
}

void gfx_drawStaticElements(void) {
  // unsigned int x;
  // unsigned int y;
  // unsigned int yChanNo;
  // char buffer [2];

  // Draw horizontal dividers
  gfx_tft.drawFastHLine(0, DIAL_ROW_HEIGHT, 800, LIGHT_GREY);
  gfx_tft.drawFastHLine(0, 480 - DIAL_ROW_HEIGHT, 800, LIGHT_GREY);

  // Draw vertical dividers
  for (byte chan = 1; chan < 8; chan++) {
    gfx_tft.drawFastVLine(chan * 100, 0, DIAL_ROW_HEIGHT, LIGHT_GREY);
    gfx_tft.drawFastVLine(chan * 100, 480 - DIAL_ROW_HEIGHT, DIAL_ROW_HEIGHT, LIGHT_GREY);
  }

  // // Draw channel numbers
  // for (byte row = 0; row < 2; row++) {
  //   y = gfx_getDialY(row);
  //   yChanNo = row == 0 ? y + OLD_DIAL_RADIUS + 3 : y - OLD_DIAL_RADIUS - 35;
  //   for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
  //     x = gfx_getDialX(channel);
  //     itoa(channel + 1, buffer, 10);
  //     gfx_drawText(x - 8, yChanNo, buffer, TEXT_MD, LIGHT_GREY);
  //   }
  // }
}

void gfx_start(void) {
  gfx_tft.fillScreen(BLACK);
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

void __gfx_log(const char *s) {
  gfx_drawText(500, 240, s);
}