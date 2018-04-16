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
#include <r_socket.h>
#include <cJSON.c>

using namespace std;

#define SZ(x) ((int)x.size())

const int kNumPCs = 1 << 21;

extern uint8_t __sancov_trace_pc_guard_8bit_counters[kNumPCs];
extern uint8_t __sancov_trace_pc_pcs[kNumPCs];

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

void err_msg(const char *msg){
    perror(msg);
    exit(1);
}

string r2cmd(R2Pipe *r2, const char *cmd) {
    char *msg = r2p_cmd(r2, cmd);
    if(msg) {
        string result(msg);
        free(msg);
        return result;
    }
}

void get_block_info(map<unsigned long long, int> &index) {
    
    string open_r2 = "r2 -q0 ", filename(getenv("FUZZBIN"));
    open_r2 += filename;
    R2Pipe *r2 = r2p_open(open_r2.c_str());
    string data;
    if(r2) {
        r2cmd(r2, "aa");
        r2cmd(r2, "s main");
        data = r2cmd(r2, "afbj");
        r2p_close(r2);
    }
    
    int jedi = 0;
    
    cJSON *root = cJSON_Parse(data.c_str());
    int arr_size = cJSON_GetArraySize(root);
    for(int i = 0; i < arr_size; i++) {
        cJSON *item = cJSON_GetArrayItem(root, i);
        if (item) {
            cJSON *ele = cJSON_Parse(cJSON_Print(item));
            cJSON *addr = cJSON_GetObjectItem(ele, "addr");
            unsigned long long val;
            sscanf(cJSON_Print(addr), "%llu", &val);
            index[val] = jedi++;
        }
    }
}

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
            err_msg("read");
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
        catch(exception &e) {
            cout << e.what() << endl;
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

    map<unsigned long long, int> index;

    get_block_info(index);

    while(true) {
        try {
            parse_line(fd);
            string line = parse_line(fd);
            bool valid = true;
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
                if(index.find(current.start()) == index.end()) break;
                int i = index[current.start()];
                __sancov_trace_pc_pcs[i] = current.start();
                __sancov_trace_pc_guard_8bit_counters[i]++;
            }
        }
        catch(exception &e) {
            cout << e.what() << endl;
            break;
        }
    }
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
