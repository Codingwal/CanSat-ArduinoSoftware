#include <iostream>

#define LAT 17
#define LON 29
#define ALT 51

int charToInt(char c)
{
    return c - '0';
}

void gpsStringToFloats(float *dest, const char *str)
{
    dest[0] = charToInt(str[LAT]) * 1000 + charToInt(str[LAT + 1]) * 100 + charToInt(str[LAT + 2]) * 10 + charToInt(str[LAT + 3]) 
    + charToInt(str[LAT + 5]) * 0.1 + charToInt(str[LAT + 6]) * 0.01 + charToInt(str[LAT + 7]) * 0.001+ charToInt(str[LAT + 8]) * 0.0001;

    dest[1] = charToInt(str[LON]) * 10000 + charToInt(str[LON + 1]) * 1000 + charToInt(str[LON + 2]) * 100 + charToInt(str[LON + 3]) * 10 + charToInt(str[LON + 4]) 
    + charToInt(str[LON + 6]) * 0.1 + charToInt(str[LON + 7]) * 0.01 + charToInt(str[LON + 8]) * 0.001+ charToInt(str[LON + 9]) * 0.0001;

    dest[2] = charToInt(str[ALT]) * 1000 + charToInt(str[ALT + 1]) * 100 + charToInt(str[ALT + 2]) * 10 + charToInt(str[ALT + 3]) 
    + charToInt(str[ALT + 5]) * 0.1 + charToInt(str[ALT + 6]) * 0.01;
}

int main()
{
    float data[3];
    gpsStringToFloats(data, "$GPGGA,202530.00,5109.0262,N,11401.8407,W,5,40,0.5,1097.36,M,-17.00,M,18,TSTR*61");
    printf("%f\n", data[0]);
    printf("%f\n", data[1]);
    printf("%f\n", data[2]);

    return 0;
}