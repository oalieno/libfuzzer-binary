#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

const int kNumPCs = 1 << 21;

extern uint8_t __sancov_trace_pc_guard_8bit_counters[kNumPCs];
extern uint8_t __sancov_trace_pc_pcs[kNumPCs];

void error(const char *msg){
    perror(msg);
    exit(1);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    printf("Size : %d\n", (int) Size);

    // 0 : read, 1 : write
    int P_IN[2], P_ERR[2];
    if(pipe(P_IN) < 0) error("pipe");
    if(pipe(P_ERR) < 0) error("pipe");
    write(P_IN[1], Data, Size);

    int pid;
    if((pid = fork()) < 0) error("fork");
    else if(pid == 0) {
        int dev_null = open("/dev/null", O_WRONLY);
        if(dup2(P_IN[0], STDIN_FILENO) < 0) error("dup2");
        if(dup2(dev_null, STDOUT_FILENO) < 0) error("dup2");
        if(dup2(P_ERR[1], STDERR_FILENO) < 0) error("dup2");
        char * const args[7] = { "/usr/bin/timeout", "5", "/usr/bin/qemu-x86_64", "-d", "in_asm", "/home/oalieno/Alien/libfuzzer-binary/test", (char *) 0 };
        execve(args[0], args, NULL);
        perror("execve");
        exit(0);
    }
    
    close(P_ERR[1]);
    while(1) {
        char ch;
        int r = read(P_ERR[0], &ch, 1);
        if(r == 1) {
            printf("%c", ch);
        }
        else break;
    }
    close(P_ERR[0]);
    
    wait(NULL);

    close(P_IN[0]);
    close(P_IN[1]);
    
    return 0;
}
