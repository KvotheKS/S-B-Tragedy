#pragma once 
#include <set>
#include <string>
#include <map>

struct label_info
{
    int end = - 1;
    std::vector<int> dep_l;
    std::vector<int> dep_arg;
};

const std::set<std::string> reserved({"SPACE", "CONST", "IF", "EQU", "ADD", "MULT", "SUB", "DIV", "INPUT", "OUTPUT", "STOP", "COPY", "LOAD", "STORE", "JMPP", "JMPN", "JMP", "JMPZ", "MACRO", "ENDMACRO"});

std::map<std::string, int> instructions({{"ADD", 1}, {"MULT", 3}, {"SUB", 2}, {"DIV",4}, {"INPUT",12}, {"OUTPUT",13}, {"STOP",14}, {"COPY",9}, {"LOAD",10} , {"STORE",11}, {"JMPP",7}, {"JMPN",6}, {"JMP",5}, {"JMPZ",8}});

std::map<std::string, label_info> label_table;

std::vector<std::vector<int>> exec_lines;