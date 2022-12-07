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
        tokenize(tokens, line, sep, spec);
        // tokens.push_back(tokenized);
        if(eof) break;
    }    
}

enum label_type
{
    SPACE, CONST, EQU, INST, ERROR
};

struct macro_info
{
    int l, r; // l eh o indice onde o nome da macro foi definida e r onde ENDMACRO foi definido
    std::vector<std::string> mapped;
    std::map<int, std::vector<std::string>> tokens
}

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
    if(!sec_text)
        err += "SECTION TEXT not declared (semantic error)\n";
    
    // for(auto& it : equ)
    //     std::cout << it.first << ' ' << it.second << std::endl;
    if(err.size())
    {
        std::cout << "Pre-Processing dropped due to errors:\n\n" << err << std::endl;
        exit(EXIT_FAILURE);
    }
}

void macro_processing(std::vector<std::vector<std::string>>& tokens, std::map<std::string, macro_info>& macros)
{
    std::string err;
    bool sec_text = false, decl = false, sec_data = false;
    int i = -1;
    for(auto& line : tokens)
    {
        i++;
        if(line.empty()) continue;
        if(line.size() == 2 && line[0] == "SECTION" )
        {
            if(line[1] == "TEXT")
                sec_text = true;
            else if(line[1] == "DATA")
                sec_data = true;
        }
        if(line.size() >= 2)
        {
            if(line[1] == "MACRO")
            {
                if(!sec_text || sec_data)
                {
                    err += LineLabel(i) + "MACROS have to be defined inside of SECTION TEXT(semantic error)\n";
                    continue;
                }

                if(!label_valid(line[0]))
                {
                    err += LineLabel(i) + "MACRO keyword has to be identified by a token that follows the rule: ([A-Z_]+[A-Z0-9_]*:) (lexical error)\n";
                    continue;
                }

                std::string tmp_lbl = line[0].substr(0, line[0].size()-1);
                auto it = macros.find(tmp_lbl);

                if(it != macros.end())
                {
                    err += LineLabel(i) + "MACRO with same id has already been defined (semantic error)\n";
                    continue;
                }

                if(reserved.find(tmp_lbl) != reserved.end())
                {
                    err += LineLabel(i) + "Labels can't use reserved tokens (lexical error)\n";
                    continue;
                }

                decl = true;
                it = macros.insert({tmp_lbl, {i, -1, {}, {}}});
                bool inval_arg = false;
                for(int z = 2; z < line.size(); z++)
                {
                    if(line[z] == ",") continue;

                    if(!macro_arg(line[z]))
                    {inval_arg = true; break;}

                    it->mapped.push_back(line[z].substr(0, line[z].size() - (line[z][line.size()-1] == ','))); // Se a gente recebe &arg, devemos retirar ','. 
                }
                if(inval_arg)
                {
                    err += LineLabel(i) + "Invalid argument format in MACRO declaration (lexical error)\n";
                    continue;
                }
            }
        }
        if(line[0] == "ENDMACRO")
        {
            if(!decl)
            {
                err += LineLabel(i) + "Unexpected ENDMACRO label has been found (semantic error)\n";
                continue;
            }
            macros[macros.size()-1].r = i;
            decl=false;
        }
        if((auto it = macros.find(line[0])) != macros.end())
        {
            if(it->r == -1)
            {
                err += LineLabel(i) + "Can't call a macro inside itself (semantic error)\n";
                continue;
            }
        }
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
    std::map<std::string, macro_info> macros;

    read_asm(tokens, "./bin.asm");
    upper_case(tokens);
    pre_proccess(tokens);
    pre_proccess_file(tokens);
}
