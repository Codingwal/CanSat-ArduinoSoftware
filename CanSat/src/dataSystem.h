#define DATA_BLOCK_START 0xFFFFFFFF // maximaler 32b Wert
#define DATA_BLOCK_END 0xFFFFFFFE   // maximaler 32b Wert minus 1

struct Vector3
{
  float x = 1;
  float y = 2;
  float z = 3;

  Vector3()
  {
  }
  Vector3(float x, float y, float z) : x(x), y(y), z(z)
  {
  }
};
struct DataBlock
{
  float temperature = 1;
  float pressure = 2;
  Vector3 acceleration = Vector3();
  Vector3 position = Vector3();
};
