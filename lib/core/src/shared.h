#define CHANNEL_COUNT 8

#define ROW_TOP 0
#define ROW_BOTTOM 1

#define PARAM_COUNT 25
#define PARAM_VOL 0
#define PARAM_PAN 1
#define PARAM_CHA_STATE 24

#define FILTER_TYPE_COUNT 8

#define ENC_ACTION_NONE 0
#define ENC_ACTION_INC 1
#define ENC_ACTION_DEC 2
#define ENC_ACTION_PRESS 3
#define ENC_ACTION_RELEASE 4

#define CHA_STATE_NORMAL 0
#define CHA_STATE_MUTED 1
#define CHA_STATE_SOLOED 2

typedef struct {
  const byte es9Id;
  const char* name;
} FilterType;

typedef byte Mix[PARAM_COUNT][CHANNEL_COUNT];

typedef struct {
  byte pIds[2];
  char chanNames[8][6];
  Mix mix;
} Scene;

const FilterType filterTypes[FILTER_TYPE_COUNT] = {
  { 0x00, "---" },
  { 0x09, "LSH" },
  { 0x0D, "BND" },
  { 0x0B, "HSH" },
  { 0x01, "LP1" },
  { 0x05, "LP2" },
  { 0x03, "HP1" },
  { 0x07, "HP2" }
};

void gfx_setup(void);
void gfx_start(void);
void gfx_clear(void);
void gfx_drawMenuItem(byte row, byte chan, const char* text[3], bool isEditable = false);
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
);
void gfx_drawParamName(byte row, const char* name);
void gfx_drawFlash(const char* msg);
void gfx_clearFlash();
void __gfx_log(const char* s);

bool eep_load(Scene *currScene, byte slot);
void eep_save(Scene *currScene, byte slot);

void es9_setup(Mix mix);
void es9_sendParam(byte paramId, byte channel, Mix mix);

void encs_setup(void);
unsigned int encs_read(void);