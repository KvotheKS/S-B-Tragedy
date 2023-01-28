#pragma once 
#include <set>
#include <string>
#include <map>

struct label_info
{
    int end = - 1;
    std::vector<int> dep_l;
    std::vector<int> dep_arg;
    int spaces = 0;
    bool is_const = false;
    int val = 0;
};

const std::set<std::string> reserved({"SPACE", "CONST", "IF", "EQU", "ADD", "MULT", "SUB", "DIV", "INPUT", "OUTPUT", "STOP", "COPY", "LOAD", "STORE", "JMPP", "JMPN", "JMP", "JMPZ", "MACRO", "ENDMACRO"});

std::map<std::string, int> instructions({{"ADD", 1}, {"MULT", 3}, {"SUB", 2}, {"DIV",4}, {"INPUT",12}, {"OUTPUT",13}, {"STOP",14}, {"COPY",9}, {"LOAD",10} , {"STORE",11}, {"JMPP",7}, {"JMPN",6}, {"JMP",5}, {"JMPZ",8}, {"INPUT_S",15}, {"INPUT_C", 16}, {"OUTPUT_S", 17}, {"OUTPUT_C", 18}});
std::map<int, std::string> instructions_32({{1, "add"}, {3, "imul"}, {2, "sub"}, {4, "idiv"}, {12, "input"}, {13, "output"}, {14, "stop"}, {9, "copy"}, {10, "load"}, {11, "store"}, {7, "ja"}, {6, "jb"}, {5, "jmp"}, {8, "jz"}, {15, "input_s"}, {16, "input_c"}, {17, "output_s"}, {18, "output_c"}});

std::map<std::string, label_info> label_table;
std::set<std::string> one_byter;

std::vector<std::vector<int>> exec_lines;