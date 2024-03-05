#include <iostream>

int charToInt(char c)
{
    return c - '0';
}

void gpsStringToFloats(float *dest, const char *str)
{
    dest[0] = charToInt(str[16]) * 10 + charToInt(str[17]);
    dest[1] = charToInt(str[18]) * 10 + charToInt(str[19]) + charToInt(str[21]) * 0.1 + charToInt(str[22]) * 0.01 + charToInt(str[23]) * 0.001;

    dest[2] = charToInt(str[29]) * 100 + charToInt(str[30]) * 10 + charToInt(str[31]);
    dest[3] = charToInt(str[32]) * 10 + charToInt(str[33]) + charToInt(str[35]) * 0.1 + charToInt(str[36]) * 0.01 + charToInt(str[37]) * 0.001;

    dest[4] = charToInt(str[55]) * 100 + charToInt(str[56]) * 10 + charToInt(str[57]) + charToInt(str[59]) * 0.1;
}

int main()
{
    float data[5];
    gpsStringToFloats(data, "$GPGGA, 123519, 4807.038, N, 01131.000, E, 1, 08, 0.9, 545.4, M, 46.9, M, , *47");
    printf("%f\n", data[0]);
    printf("%f\n", data[1]);
    printf("%f\n", data[2]);
    printf("%f\n", data[3]);
    printf("%f\n", data[4]);

    return 0;
}