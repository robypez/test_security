typedef struct
{
  int pin;
  int state;
  int volumetric;
  int perimetral;
  unsigned long int alarmed_timestamp;
  PGM_P name;
  bool enabled;
  bool alarmed;
} sensor;

typedef struct
{
  uint8_t uid[4];
  char uid_name[10];
} token;
