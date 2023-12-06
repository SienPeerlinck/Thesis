#include "utils.h"
#include "opt/printf-stdarg.h"

void hex_dump(char *p, unsigned int n) {
    for (unsigned int i = 0; i < n; i++, p++) {
        if (i % 4 == 0) printf(" ");
        printf("%02x ", *p);
    }
    printf("\n");
}

unsigned char to_bcd(unsigned char x) {
    return ((x / 10) << 4) | (x % 10);
}

unsigned char from_bcd(unsigned char x) {
    return 10 * ((x >> 4) & 0x0f) + (x & 0x0f);
}