#pragma once 
#include <set>
#include <string>
#include <map>

struct label_info
{
    int end;
    std::vector<int> dep_l;
    std::vector<int> dep_arg;
};

const std::set<std::string> reserved({"SPACE", "CONST", "IF", "EQU", "ADD", "MULT", "SUB", "DIV", "INPUT", "OUTPUT", "STOP", "COPY", "LOAD", "STORE", "JMPP", "JMPN", "JMP", "JMPZ", "MACRO", "ENDMACRO"});

const std::set<std::string> instructions({"ADD", "MULT", "SUB", "DIV", "INPUT", "OUTPUT", "STOP", "COPY", "LOAD", "STORE", "JMPP", "JMPN", "JMP", "JMPZ"});

std::map<std::string, label_info> label_table;

std::vector<std::vector<int>> exec_lines;