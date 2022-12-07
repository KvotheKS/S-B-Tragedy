#include "./aux_string.h"
#include "./global_vars.h"
#include <cstdio>

std::vector<char> sep({' '}), spec({':'});

void read_asm(std::vector<std::vector<std::string>>& tokens, const char* file)
{
    std::string line;
    std::ifstream fd(file);
    bool eof = false;
    
    int i = 0;
    for(eof = std::getline(fd, line).eof();;eof = std::getline(fd, line).eof(), i++)
    {
        auto tokenized = tokenize(line, sep, spec);
        tokens.push_back(tokenized);
        if(eof) break;
    }    
}

enum label_type
{
    SPACE, CONST, EQU, INST, ERROR
};

void pre_proccess(std::vector<std::vector<std::string>>& tokens)
{
    std::map<std::string, std::string> equ;
    std::string err;
    bool sec_text = false;
    
    int i = -1; 

    for(auto& line : tokens)
    {
        i++;
        
        if(line.size() == 0) continue;

        if(sec_text)
        {
            auto it = equ.end();

            if(line.size() <= 1) continue;

            if(line[1] == "EQU")
            {
                err += LineLabel(i) + "EQU directive can't be declared after SECTION TEXT (semantic error)\n";
                continue;
            }
            
            if(line.size() == 2 && line[0] == "IF")
            {
                if((it = equ.find(line[1])) == equ.end()) // IF INEXISTENT_FLAG
                {
                    err += LineLabel(i) + "FLAG not found (semantic error)\n";
                    continue;
                }
                else
                {
                    short int flg = std::stoi(it->second);
                    int j;

                    for(j = i + 1; j < tokens.size(); j++)
                    {
                        if(!tokens[j].empty())
                            break;
                    }

                    // if(j == tokens.size()) // IF FLAG \ eof
                    // {
                    //     err += LineLabel(i) + "IF directive has to be followed by an instruction (semantic error)\n";
                    //     continue;
                    // }
                    
                    if(flg == 0)
                        tokens[j].clear();

                    tokens[i].clear();
                }
            }

            if(line.size() >= 3 && (line[1] == "SPACE" || line[1] == "CONST"))
            {
                // std::cout << "WE IN " << line[0] << std::endl;
                auto it = equ.find(line[2]);
                
                if(it != equ.end())
                    line[2] = it->second;
            }
            
            continue;
        }

        // if(line.size() <= 1)
        // {
        //     err += LineLabel(i) + "Unexpected number of tokens in line. Expected 3 (syntatic error)\n";
        //     continue;
        // }

        if(line.size() == 2 && line[0] == "SECTION" && line[1] == "TEXT") sec_text = true;
        
        else if(line.size() >= 2 && line[0][line[0].size()-1] == ':')
        {
            if(line[1] == "EQU")
            {
                if(!label_valid(line[0]))
                {
                    err += LineLabel(i) + "EQU label has to follow the following regex: ([A-Z_]+[A-Z0-9_]*:). (lexical error)\n";
                    continue;
                }

                std::string lbl_name = line[0].substr(0,line[0].size()-1);
                
                if(reserved.find(lbl_name) != reserved.end())
                {
                    err += LineLabel(i) + "Label can't use reserved token (lexical error)\n";
                    continue;
                }

                if(line.size() != 3)
                {    
                    err += LineLabel(i) + "Wrong number of tokens in line. Expected 3 (syntatic error)\n";
                    continue;
                }

                auto pnum = to_num(line[2]);
                
                if(pnum)
                {
                    err += LineLabel(i) + "Invalid EQU token. Expected number (lexical error)\n";
                    continue;
                }

                if(equ.find(line[0]) != equ.end())
                {
                    err += LineLabel(i) + "EQU label already defined (semantic error)\n";
                    continue;
                }
                equ.insert({lbl_name, std::to_string((short int)std::stoi(line[2]))});
                tokens[i].clear();
            }    
        }
    }
    // for(auto& it : equ)
    //     std::cout << it.first << ' ' << it.second << std::endl;
    if(err.size())
    {
        std::cout << "Pre-Processing dropped due to errors:\n\n" << err << std::endl;
        exit(EXIT_FAILURE);
    }
}

void pre_proccess_file(std::vector<std::vector<std::string>>& tokens)
{
    std::ofstream fout("./bin.PRE");

    for(auto& line : tokens)
    {
        if(line.empty()) continue;

        for(auto& token : line)
            fout << token << ' ';
        fout << '\n';
    }
}

int main()
{
    std::vector<std::vector<std::string>> tokens;
    
    read_asm(tokens, "./bin.asm");
    upper_case(tokens);
    pre_proccess(tokens);
    pre_proccess_file(tokens);
}