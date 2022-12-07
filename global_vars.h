#pragma once 
#include <set>
#include <string>
#include <map>

struct instr_info
{};

const std::set<std::string> reserved({"SPACE", "CONST", "IF", "EQU", "ADD", "MULT", "SUB", "DIV", "INPUT", "OUTPUT", "STOP", "COPY", "LOAD", "STORE", "JMPP", "JMPN", "JMP", "JMPZ"});
const std::map<std::string, instr_info> instrs;