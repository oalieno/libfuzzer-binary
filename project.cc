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

#define RED "\033[91m"
#define NORMAL "\033[0m"

using namespace std;

void err_msg(const char *msg) {
    perror(msg);
    exit(1);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    char * const filename = getenv("FUZZBIN");
    if(filename == NULL) {
        cout << RED << "Usage : FUZZBIN=/absolute/path/to/your/filename/ ./fuzzer" << NORMAL << endl;
        exit(0);
    }

    int P_IN[2], P_ERR[2];
    if(pipe(P_IN) < 0) err_msg("pipe");
    if(pipe(P_ERR) < 0) err_msg("pipe");
    if(Size) write(P_IN[1], Data, Size);
    close(P_IN[1]);

    int pid;
    if((pid = fork()) < 0) err_msg("fork");
    else if(pid == 0) {
        int dev_null = open("/dev/null", O_WRONLY);
        if(dup2(P_IN[0], STDIN_FILENO) < 0) err_msg("dup2");
        if(dup2(dev_null, STDOUT_FILENO) < 0) err_msg("dup2");
        if(dup2(P_ERR[1], STDERR_FILENO) < 0) err_msg("dup2");
        const char * args[5] = { "/usr/bin/qemu-x86_64", "-d", "in_asm", filename, (char *) 0 };
        execv(args[0], const_cast<char * const *>(args));
        perror("execve");
        exit(0);
    }

    close(P_ERR[1]);
    parse(P_ERR[0]);

    int status;
    waitpid(pid, &status, 0);
    if(not WIFEXITED(status)) {
        cout << RED << "Error status : " << status << NORMAL << endl;
        if(WIFSIGNALED(status)) {
            cout << RED << "Signal : " << strsignal(WTERMSIG(status)) << NORMAL << endl;
        }
        exit(0);
    }

    return 0;
}
