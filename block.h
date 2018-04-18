#pragma once

#include <string>
#include <map>
#include <r_socket.h>
#include <cJSON.c>

using namespace std;

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
