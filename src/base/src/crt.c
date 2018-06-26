#include <base/sys/unistd.h>

extern int crt_call_main(int argc, char *argv[]);

void _start() {
    char *args[] = {""};
    int ret = crt_call_main(1, args);
    exit();
}
