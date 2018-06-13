#include <stdio.h>

void fn(int x) {
    printf("hi %d\n", x);
}

int main() {
    int x; scanf("%d", &x);
    void (*fp)(int) = &fn;
    fp(x);
    return 0;
}
