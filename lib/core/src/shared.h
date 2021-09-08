#define CHANNEL_COUNT 8

#define ROW_TOP 0
#define ROW_BOTTOM 1

#define PARAM_COUNT 24
#define PARAM_VOL 0
#define PARAM_PAN 1

#define PARAM_KIND_DEFAULT 0
#define PARAM_KIND_PAN 1
#define PARAM_KIND_FILTER_TYPE 2

#define FILTER_TYPE_COUNT 8

typedef struct {
  const char* name;
  const byte displayType;
} Param;

typedef struct {
  const byte es9Id;
  const char* name;
} FilterType;

const Param params[PARAM_COUNT] = {
  { "VOL" },
  { "PAN", PARAM_KIND_PAN },
  { "EQ1 TYPE", PARAM_KIND_FILTER_TYPE },
  { "EQ1 FREQ" },
  { "EQ1 GAIN" },
  { "EQ1 Q" },
  { "EQ2 TYPE", PARAM_KIND_FILTER_TYPE },
  { "EQ2 FREQ" },
  { "EQ2 GAIN" },
  { "EQ2 Q" },
  { "EQ3 TYPE", PARAM_KIND_FILTER_TYPE },
  { "EQ3 FREQ" },
  { "EQ3 GAIN" },
  { "EQ3 Q" },
  { "EQ4 TYPE", PARAM_KIND_FILTER_TYPE },
  { "EQ4 FREQ" },
  { "EQ4 GAIN" },
  { "EQ4 Q" },
  { "AUX1" },
  { "AUX1 PAN", PARAM_KIND_PAN },
  { "AUX2" },
  { "AUX2 PAN", PARAM_KIND_PAN },
  { "AUX3" },
  { "AUX3 PAN", PARAM_KIND_PAN }
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

void es9_setup(E18State state);
void es9_sendParam(byte paramId, byte channel, E18State state);

void encs_setup(void);
void encs_read(void);
int encs_newIndex;
int encs_newAction;