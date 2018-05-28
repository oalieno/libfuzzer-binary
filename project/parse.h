#pragma once

#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <exception>

#include "block.h"
#include "struct.h"

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
        string line = parse_line(fd);
        result.push_back(line);
        if(line.find(text) != string::npos) {
            break;
        }
    }
    return result;
}

void parse(int fd) {
    try {
        parse_line_until(fd, "entry");
        map<unsigned long long, basic_block> index;
        get_block_info(index);
        while(true) {
            parse_lines(fd, 2);
            Segment current;
            bool valid = true;
            while(true) {
                string line = parse_line(fd);
                if(line.empty()) break;
                unsigned long long address = stoull(line.substr(0, 18), nullptr, 16);
                if(current.address.empty()) {
                    map<unsigned long long, basic_block>::iterator it = index.upper_bound(address);
                    it--;
                    if(it == index.end() || it->first + it->second.size < address) valid = false;
                }
                current.address.push_back(address);
            }
            if(not valid) continue;
            int i = index[current.start()].index;
            __sancov_trace_pc_pcs[i] = current.start();
            __sancov_trace_pc_guard_8bit_counters[i]++;
        }
    }
    catch(exception & e) {
        return;
    }
}
