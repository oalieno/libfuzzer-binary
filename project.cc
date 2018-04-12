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

using namespace std;

#define SZ(x) ((int)x.size())

const int kNumPCs = 1 << 21;

extern uint8_t __sancov_trace_pc_guard_8bit_counters[kNumPCs];
extern uint8_t __sancov_trace_pc_pcs[kNumPCs];

void error(const char *msg){
    perror(msg);
    exit(1);
}

struct Segment {
    vector<unsigned long long> address;
    unsigned long long start() {
        if(address.empty()) return 0;
        return *address.begin();
    }
    unsigned long long end() {
        if(address.empty()) return 0;
        return *address.rbegin();
    }
};

string parse_line(int fd) {
    string result;
    while(1) {
        char ch;
        int r = read(fd, &ch, 1);
        if(r == 1) {
            if(ch == '\n') break;
            else result += ch;
        }
        else if(r == 0) {
            throw exception();
        }
        else {
            error("read");
        }
    }
    return result;
}

vector<string> parse_lines(int fd, int n) {
    vector<string> result;
    for(int i = 0; i < n; i++) {
        result.push_back(parse_line(fd));
    }
    return result;
}

vector<string> parse_line_until(int fd, string text) {
    vector<string> result;
    while(true) {
        try {
            string line = parse_line(fd);
            result.push_back(line);
            if(line.find(text) != string::npos) {
                break;
            }
        }
        catch (exception &e) {
            break;
        }
    }
    return result;
}

void parse(int fd) {
    Segment main;

    parse_line_until(fd, "start");

    string line = parse_line(fd);
    assert(SZ(line) >= 16 + 16 + 1);
    main.address.push_back(stoull(line.substr(0, 16), nullptr, 16));
    main.address.push_back(stoull(line.substr(17, 16), nullptr, 16));

    parse_line_until(fd, "entry");

    int jedi = 0;
    map<unsigned long long, int> index;
    while(true) {
        try {
            parse_line(fd);
            string line = parse_line(fd);
            bool valid = false; if(line.find("main") != string::npos) valid = true;
            Segment current;
            while(true) {
                string line = parse_line(fd);
                if(line.empty()) break;
                unsigned long long address = stoull(line.substr(0, 18), nullptr, 16);
                if(main.start() <= address and address <= main.end()) {
                    current.address.push_back(address);
                }
            }
            if(valid) {
                if(index.find(current.start()) == index.end()) {
                    index[current.start()] = jedi++;
                }
                int i = index[current.start()];
                __sancov_trace_pc_pcs[i] = current.start();
                __sancov_trace_pc_guard_8bit_counters[i]++;
            }
        }
        catch (exception &e) {
            break;
        }
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
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
