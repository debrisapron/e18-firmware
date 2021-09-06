#define E18_VERSION 2

#define CHANNEL_COUNT 8

#define ROW_TOP 0
#define ROW_BOTTOM 1

#define PARAM_COUNT 8
#define PARAM_VOL 0
#define PARAM_PAN 1
#define PARAM_EQ1_TYPE 2
#define PARAM_AUX1 6
#define PARAM_AUX1_PAN 7

#define FILTER_TYPE_COUNT 8

typedef struct {
  const char* name;
} Param;

typedef struct {
  const byte es9Id;
  const char* name;
} FilterType;

const Param params[PARAM_COUNT] = {
  { "VOL" },
  { "PAN" },
  { "EQ1 TYPE" },
  { "EQ1 FREQ" },
  { "EQ1 GAIN" },
  { "EQ1 Q" },
  { "AUX1" },
  { "AUX1 PAN" }
};

const FilterType filterTypes[FILTER_TYPE_COUNT] = {
  { 0x0, "---" },
  { 0x9, "LSH" },
  { 0xD, "BND" },
  { 0xB, "HSH" },
  { 0x1, "LP1" },
  { 0x5, "LP2" },
  { 0x3, "HP1" },
  { 0x7, "HP2" }
};

typedef byte E18State[PARAM_COUNT][CHANNEL_COUNT];