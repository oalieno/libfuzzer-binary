#pragma once

#include <string>
#include <map>
#include <r_socket.h>
#include <cJSON.c>

using namespace std;

string r2cmd(R2Pipe *r2, string cmd) {
    char *msg = r2p_cmd(r2, cmd.c_str());
    if(msg) {
        string result(msg);
        free(msg);
        return result;
    }
    return NULL;
}

void get_block_info(map<unsigned long long, int> &index) {
    string open_r2 = "r2 -q0 ", filename(getenv("FUZZBIN"));
    open_r2 += filename;
    R2Pipe *r2 = r2p_open(open_r2.c_str());

    int counter = 0;

    string func_list;
    if(r2) {
        r2cmd(r2, "e anal.jmptbl = true");
        r2cmd(r2, "aaa");
        func_list = r2cmd(r2, "aflj");
    }

    const int whitelist_size = 9;
    string whitelist[whitelist_size] = {
        "imp.",
        "_init",
        "entry",
        "deregister_tm_clones",
        "register_tm_clones",
        "__do_global_dtors_aux",
        "__libc_csu_init",
        "__libc_csu_fini",
        "_fini"
    };

    cJSON *func_root = cJSON_Parse(func_list.c_str());
    int func_list_arr_size = cJSON_GetArraySize(func_root);
    for(int j = 0; j < func_list_arr_size; j++) {
        cJSON *func_item = cJSON_GetArrayItem(func_root, j);
        if (func_item) {
            cJSON *func_ele = cJSON_Parse(cJSON_Print(func_item));
            cJSON *func_cJson_name = cJSON_GetObjectItem(func_ele, "name");
            char *func_name = cJSON_Print(func_cJson_name);
            string str_func_name = string(func_name);

            bool banned = false;
            for(int k = 0; k < whitelist_size; k++) {
                if(str_func_name.find(whitelist[k]) != string::npos) {
                    banned = true;
                    break;
                }
            }

            if(!banned) {
                string data;
                if(r2) {
                    r2cmd(r2, "s " + str_func_name.substr(1, str_func_name.size() - 2));
                    data = r2cmd(r2, "afbj");
                }

                cJSON *root = cJSON_Parse(data.c_str());
                int arr_size = cJSON_GetArraySize(root);
                for(int i = 0; i < arr_size; i++) {
                    cJSON *item = cJSON_GetArrayItem(root, i);
                    if (item) {
                        cJSON *ele = cJSON_Parse(cJSON_Print(item));
                        cJSON *addr = cJSON_GetObjectItem(ele, "addr");
                        unsigned long long val;
                        sscanf(cJSON_Print(addr), "%llu", &val);
                        index[val] = counter++;
                    }
                }
            }
        }
    }

    r2p_close(r2);
}
