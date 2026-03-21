#include <stdio.h>

int main(void) {
    unsigned long long x = 0;
    if (scanf("%llu", &x) != 1) {
        return 0;
    }
    printf("%llu\n", x + 1ULL);
    return 0;
}

