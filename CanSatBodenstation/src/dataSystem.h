#define DATA_BLOCK_START 0xFFFFFFFF     // maximaler 32b Wert
#define DATA_BLOCK_END 0xFFFFFFFE       // maximaler 32b Wert minus 1
#define CSV_INVALID_VALUE 0xFFFFFFFF    // maximaler 32b Wert

struct Vector3
{
  float x = CSV_INVALID_VALUE;
  float y = CSV_INVALID_VALUE;
  float z = CSV_INVALID_VALUE;
};
struct DataBlock
{
  float temperature = CSV_INVALID_VALUE;
  float pressure = CSV_INVALID_VALUE;
  Vector3 acceleration = Vector3();
  Vector3 position = Vector3();
};
