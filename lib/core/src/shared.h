#define CHANNEL_COUNT 8

#define ROW_TOP 0
#define ROW_BOTTOM 1

#define PARAM_COUNT 8
#define PARAM_VOL 0
#define PARAM_PAN 1
#define PARAM_EQ1_TYPE 2
#define PARAM_EQ1_FREQ 3
#define PARAM_EQ1_GAIN 4
#define PARAM_EQ1_Q 5
#define PARAM_AUX1 6
#define PARAM_AUX1_PAN 7

// #define PARAM_KIND_GAIN 0
// #define PARAM_KIND_PAN 1
// #define PARAM_KIND_EQ_TYPE 2
// #define PARAM_KIND_EQ_FREQ 3
// #define PARAM_KIND_EQ_GAIN 4
// #define PARAM_KIND_EQ_Q 4

#define FILTER_TYPE_COUNT 8

typedef struct {
  const char* name;
  // const byte kind;
} Param;

typedef struct {
  const byte es9Id;
  const char* name;
} FilterType;

const Param params[PARAM_COUNT] = {
  { "VOL" }, // PARAM_KIND_GAIN },
  { "PAN" }, // PARAM_KIND_PAN },
  { "EQ1 TYPE" }, // PARAM_KIND_EQ_TYPE },
  { "EQ1 FREQ" }, // PARAM_KIND_EQ_FREQ },
  { "EQ1 GAIN" }, // PARAM_KIND_EQ_GAIN },
  { "EQ1 Q" }, // PARAM_KIND_EQ_Q },
  { "AUX1" }, // PARAM_KIND_GAIN },
  { "AUX1 PAN" }, // PARAM_KIND_PAN }
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
void gfx_drawDial(byte row, byte channel, bool isScalar, byte oldValue, byte newValue, const char* displayValue);
void gfx_drawParamName(byte row, const char* name);

void eep_load(E18State state, byte rowParams[2]);
void eep_save(E18State state, byte rowParams[2]);

void es9_setup(void);
void es9_setAllParams(E18State state);
void es9_setParam(byte paramId, byte channel, E18State state);

void encs_setup(void);
void encs_read(void);
int encs_newIndex;
int encs_newAction;