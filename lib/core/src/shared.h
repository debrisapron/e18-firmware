#define CHANNEL_COUNT 8

#define ROW_TOP 0
#define ROW_BOTTOM 1

#define PARAM_COUNT 25
#define PARAM_VOL 0
#define PARAM_PAN 1
#define PARAM_MUTE 24

#define PARAM_KIND_DEFAULT 0
#define PARAM_KIND_PAN 1
#define PARAM_KIND_FILTER_TYPE 2
#define PARAM_KIND_FILTER_FREQ 3
#define PARAM_KIND_FILTER_GAIN 4
#define PARAM_KIND_FILTER_Q 5
#define PARAM_KIND_MUTE 6

#define FILTER_TYPE_COUNT 8

#define ENC_ACTION_NONE 0
#define ENC_ACTION_INC 1
#define ENC_ACTION_DEC 2
#define ENC_ACTION_PRESS 3
#define ENC_ACTION_RELEASE 4

typedef struct {
  const char* name;
  const byte kind;
} Param;

typedef struct {
  const byte es9Id;
  const char* name;
} FilterType;

const Param params[PARAM_COUNT] = {
  { "VOL" },
  { "PAN", PARAM_KIND_PAN },
  { "EQ1 TYPE", PARAM_KIND_FILTER_TYPE },
  { "EQ1 FREQ", PARAM_KIND_FILTER_FREQ },
  { "EQ1 GAIN", PARAM_KIND_FILTER_GAIN },
  { "EQ1 Q", PARAM_KIND_FILTER_Q },
  { "EQ2 TYPE", PARAM_KIND_FILTER_TYPE },
  { "EQ2 FREQ", PARAM_KIND_FILTER_FREQ },
  { "EQ2 GAIN", PARAM_KIND_FILTER_GAIN },
  { "EQ2 Q", PARAM_KIND_FILTER_Q },
  { "EQ3 TYPE", PARAM_KIND_FILTER_TYPE },
  { "EQ3 FREQ", PARAM_KIND_FILTER_FREQ },
  { "EQ3 GAIN", PARAM_KIND_FILTER_GAIN },
  { "EQ3 Q", PARAM_KIND_FILTER_Q },
  { "EQ4 TYPE", PARAM_KIND_FILTER_TYPE },
  { "EQ4 FREQ", PARAM_KIND_FILTER_FREQ },
  { "EQ4 GAIN", PARAM_KIND_FILTER_GAIN },
  { "EQ4 Q", PARAM_KIND_FILTER_Q },
  { "AUX1" },
  { "AUX1 PAN", PARAM_KIND_PAN },
  { "AUX2" },
  { "AUX2 PAN", PARAM_KIND_PAN },
  { "AUX3" },
  { "AUX3 PAN", PARAM_KIND_PAN },
  { "MUTE", PARAM_KIND_MUTE }
};

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

typedef byte E18State[PARAM_COUNT][CHANNEL_COUNT];

void gfx_setup(void);
void gfx_start(void);
void gfx_drawDial(byte row, byte channel, byte oldValue, byte newValue, const char* displayValue, bool isScalar, bool isDisabled, bool isMuted);
void gfx_drawParamName(byte row, const char* name);
void __gfx_click(bool isDown);

void eep_load(E18State state, byte rowParams[2]);
void eep_save(E18State state, byte rowParams[2]);

void es9_setup(E18State state);
void es9_sendParam(byte paramId, byte channel, E18State state);

void encs_setup(void);
unsigned int encs_read(void);