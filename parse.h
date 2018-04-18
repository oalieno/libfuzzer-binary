#pragma once

#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <exception>

#include "block.h"

using namespace std;

#define SZ(x) ((int)x.size())

extern void err_msg(const char *);

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
