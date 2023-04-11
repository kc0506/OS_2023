#include "kernel/types.h"
//
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    // char *p = (char *)(64ull << 12);

    // *p = 1;

    // int x = 0, y;
    // printf("%p\n", &x);
    // printf("%p\n", &y);


    // printf("first 512 bytes:\n");
    // write(1, 0, 512);
    // printf("\n");

    vmprint();
    exit(0);
}
