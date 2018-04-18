#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <exception>

#include "parse.h"

using namespace std;

void err_msg(const char *msg){
    perror(msg);
    exit(1);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    // 0 : read, 1 : write
    int P_IN[2], P_ERR[2];
    if(pipe(P_IN) < 0) err_msg("pipe");
    if(pipe(P_ERR) < 0) err_msg("pipe");
    write(P_IN[1], Data, Size);

    int pid;
    if((pid = fork()) < 0) err_msg("fork");
    else if(pid == 0) {
        int dev_null = open("/dev/null", O_WRONLY);
        if(dup2(P_IN[0], STDIN_FILENO) < 0) err_msg("dup2");
        if(dup2(dev_null, STDOUT_FILENO) < 0) err_msg("dup2");
        if(dup2(P_ERR[1], STDERR_FILENO) < 0) err_msg("dup2");
        char * const args[7] = { "/usr/bin/timeout", "5", "/usr/bin/qemu-x86_64", "-d", "in_asm", getenv("FUZZBIN"), (char *) 0 };
        execve(args[0], args, NULL);
        perror("execve");
        exit(0);
    }
    
    close(P_ERR[1]);
    parse(P_ERR[0]);
    close(P_ERR[0]);
    
    int status;
    waitpid(pid, &status, 0);
    if(not WIFEXITED(status)) {
        exit(0);
    }

    close(P_IN[0]);
    close(P_IN[1]);
    
    return 0;
}
