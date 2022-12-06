#include "./aux_string.h"
#include <cstdio>

std::vector<char> sep({' '}), spec({':'});

void read_asm(std::vector<std::vector<std::string>>& tokens, const char* file)
{
    std::string line;
    std::ifstream fd(file);

    while(!std::getline(fd, line).eof())
    {
        auto tokenized = tokenize(line, sep, spec);
        tokens.push_back(tokenized);    
    }    
}

enum label_type
{
    SPACE, CONST, EQU, INST, ERROR
};

void pre_proccess(std::vector<std::vector<std::string>& tokens)
{
    std::map<std::string, std::string> equ;
    std::vector<int> deletable;
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

            if(line.size() == 2 && line[1] == "IF")
            {
                if((it = equ.find(line[2])) == equ.end())
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

                    if(j == tokens.size())
                    {
                        err += LineLabel(i) + "IF directive has to be followed by an instruction (semantic error)\n";
                        continue;
                    }

                    if(flg != 0)
                        tokens[j].clear();

                    tokens[i].clear();
                }
            }

            if(line.size() == 3 && (line[1] == "SPACE" || line[1] == "CONST"))
            {
                auto it = equ.find(line[2]);
                
                if(it != equ.end())
                    line[1] = it->second;
            }

            continue;
        }

        if(line.size() == 1)
        {
            err += LineLabel(i) + "Unexpected number of tokens in line. Expected 3 (syntatic error)\n";
            continue;
        }

        if(line[0] == "SECAO" && line[1] == "TEXTO") sec_text = true;
        
        else if(line[0][line[0].size()-1] == ':')
        {
            if(line[1] == "EQU")
            {
                if(!label_valid(line[0]))
                {
                    err += LineLabel(i) + "EQU label has to follow the following regex: ([A-Z]+[A-Z0-9]*:). (lexical error)\n";
                    continue;
                }

                if(line.size() != 3)
                {    
                    err += LineLabel(i) + "Unexpected number of tokens in line. Expected 3 (syntatic error)\n";
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
                deletable.push_back(i);
                equ.insert({line[0].substr(0,line[0].size()-1), std::to_string((short int)std::stoi(line[2]))});
            }    
        }
    }

    if(err)
    {
        std::cout << "Pre-Processing dropped due to errors:\n\n" << err << std::endl;
        exit(EXIT_FAILURE);
    }

    for(auto i : deletable)
        tokens[i].clear();
}

int main()
{
    
    std::vector<std::vector<std::string>> tokens;
    read_asm(tokens, "./bin.asm");
    upper_case(tokens);
    for(auto& line : tokens)
    {
        for(auto& token : line)
            std::cout << token << ' ';
        std::cout << std::endl;
    }
}