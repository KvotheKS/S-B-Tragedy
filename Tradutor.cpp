#include "./aux_string.h"
#include "./global_vars.h"
#include <cstdio>
#include <iterator>

std::vector<char> sep({' '}), spec({':'}), spec_tk({'+'});

void read_asm(std::vector<std::vector<std::string>>& tokens, const char* file)
{
    std::string line;
    std::ifstream fd(file);
    bool eof = false;
    
    int i = 0;
    for(eof = std::getline(fd, line).eof();;eof = std::getline(fd, line).eof(), i++)
    {
        if(line[line.size()-1] == '\r') line.resize(line.size()-1);
        tokenize(tokens, line, sep, spec, spec_tk);
        if(eof) break;
    }    
}

struct macro_info
{
    int l, r; // l eh o indice onde o nome da macro foi definida e r onde ENDMACRO foi definido
    std::vector<std::string> mapped;
    std::map<int, std::vector<std::string>> tokens;
};

void pre_proccess(std::vector<std::vector<std::string>>& tokens)
{
    std::map<std::string, std::string> equ;
    std::set<std::string> label_byte;
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
                    auto [flg, errflg] = to_num(it->second);
                    int j;

                    for(j = i + 1; j < tokens.size(); j++)
                    {
                        if(!tokens[j].empty())
                            break;
                    }

                    if(flg == 0)
                        tokens[j].clear();

                    tokens[i].clear();
                }
            }

            if(line.size() >= 3 && (line[1] == "SPACE" || line[1] == "CONST"))
            {
                auto it = equ.find(line[2]);
                if(label_byte.find(line[2]) != label_byte.end())
                    one_byter.insert(line[0].substr(0,line[0].size()-1));
                if(it != equ.end())
                    line[2] = it->second;
            }
            
            continue;
        }

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
                
                

                if(line.size() != 3)
                {    
                    err += LineLabel(i) + "Wrong number of tokens in line. Expected 3 (syntatic error)\n";
                    continue;
                }

                auto [t_num, pnum] = to_num(line[2]);
                
                if(!pnum)
                {
                    err += LineLabel(i) + std::to_string(t_num) + "Invalid EQU token. Expected number (lexical error)\n";
                    continue;
                }

                if(equ.find(line[0]) != equ.end())
                {
                    err += LineLabel(i) + "EQU label already defined (semantic error)\n";
                    continue;
                }
                if(line[2][0] == '\'') label_byte.insert(lbl_name);
                equ.insert({lbl_name, std::to_string((int)t_num)});
                tokens[i].clear();
            }    
        }
    }
    if(!sec_text)
        err += "SECTION TEXT not declared (semantic error)\n";
    
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
    std::map<std::string, macro_info>::iterator curr;
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
        if(line[0] == "MACRO")
        {
            err += LineLabel(i) + "MACRO has to be named with a label (syntatic error)\n";
            continue;
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

                

                decl = true;
                it = macros.insert({tmp_lbl, {i, -1, std::vector<std::string>(), std::map<int,std::vector<std::string>>()}}).first;
                curr = it;
                bool inval_arg = false;
                for(int z = 2; z < line.size(); z++)
                {
                    if(line[z] == ",") continue;
                    if(!macro_arg(line[z]))
                    {inval_arg = true; break;}
                    it->second.mapped.push_back(line[z].substr(0, line[z].size() - (line[z][line[z].size()-1] == ','))); // Se a gente recebe &arg, devemos retirar ','. 
                }
                if(inval_arg)
                {
                    err += LineLabel(i) + "Invalid argument format in MACRO declaration (syntatic error)\n";
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
            curr->second.r = i;
            decl=false;
        }
        int l = 0;
        auto it = macros.end();
        bool inval_arg = false;
        if((it = macros.find(line[0])) != macros.end() || ((line.size()>=2) && (l=1) && (it = macros.find(line[1])) != macros.end()))
        {
            if(it->second.r == -1)
            {
                err += LineLabel(i) + "Can't call a macro inside itself (semantic error)\n";
                continue;
            }
      
            auto jt = it->second.tokens.insert({i, {}}).first;

            for(int z = l + 1; z < line.size(); z++)
            {
                if(line[z] == ",") continue;

                if(!call_arg(line[z]))
                {inval_arg = true; break;}

                jt->second.push_back(line[z].substr(0, line[z].size() - (line[z][line[z].size()-1] == ',')));
            }
            if(inval_arg)
            {
                err += LineLabel(i) + "Invalid argument format in MACRO call (syntatic error)\n";
                continue;
            }
            if(jt->second.size() != it->second.mapped.size())
            {
                err += LineLabel(i) + "Invalid argument number for MACRO, got " + std::to_string(jt->second.size()) + " expected " + std::to_string(it->second.mapped.size()) + " (semantic error)\n";
                continue;
            }
            line.erase(line.begin() + l, line.end());
        }
    }
    if(!sec_text)
        err += "SECTION TEXT not declared (semantic error)\n";
    if(decl)
        err += "Missing ENDMACRO for MACRO at " + LineLabel(std::prev(macros.end())->second.l) + "(semantic error)\n";

    if(err.size())
    {
        std::cout << "Macro Processing dropped due to errors:\n\n" << err << std::endl;
        exit(EXIT_FAILURE);
    }
}

bool push_arg(std::string& err, std::vector<std::string> vstr, int tkline)
{
    if(vstr.size() == 1)
    {
        auto it = label_table.find(vstr[0]);
        if(it == label_table.end())
        {
            label_table.insert({vstr[0], {-1, {(int)exec_lines.size()-1}, {(int)exec_lines[exec_lines.size()-1].size()}}});
            exec_lines[exec_lines.size()-1].push_back(0);
        }

        else if(it->second.end == -1)
        {
            exec_lines[exec_lines.size()-1].push_back(0);
            auto& lbl_tmp = label_table[vstr[0]];
            lbl_tmp.dep_l.push_back(exec_lines.size()-1);
            lbl_tmp.dep_arg.push_back(exec_lines[exec_lines.size()-1].size()-1);
        }

        else
            exec_lines[exec_lines.size()-1].push_back(it->second.end);
        return true;
    }
    else if(vstr.size() == 3)
    {
        if(vstr[1] != "+")
        {
            err += LineLabel(tkline) + "Token + expected (syntatic error)\n";
            return false;
        }
        auto [t_num, b_flag] = to_num(vstr[2]);
        if(!b_flag)
        {
            err += LineLabel(tkline) + "Expected valid number (lexical error)\n";
            return false;
        }
        
        auto it = label_table.find(vstr[0]);
        
        if(it == label_table.end())
        {
            label_table.insert({vstr[0], {-1, {(int)exec_lines.size()-1}, {(int)exec_lines[exec_lines.size()-1].size()}}});
            exec_lines[exec_lines.size()-1].push_back(t_num);
        }
        else if(it->second.end == -1)
        {
            exec_lines[exec_lines.size()-1].push_back(t_num);

            auto& lbl_tmp = label_table[vstr[0]];
            lbl_tmp.dep_l.push_back(exec_lines.size()-1);
            lbl_tmp.dep_arg.push_back(exec_lines[exec_lines.size()-1].size()-1);
        }
        else
            exec_lines[exec_lines.size()-1].push_back(it->second.end + t_num);
        
        return true;
    }
    err += LineLabel(tkline) + "Unexpected number of tokens (syntatic error)\n";
    return false;
}

bool push_label(std::string& lbl, int end)
{
    if(lbl[lbl.size()-1]==':')
        lbl.resize(lbl.size()-1);

    auto it = label_table.find(lbl);
    if(it != label_table.end())
    {
        if(it->second.end != -1)
            return false;
        
        it->second.end = end;

        for(int i = 0; i < it->second.dep_l.size(); i++)
            exec_lines[it->second.dep_l[i]][it->second.dep_arg[i]] += end; 
        
        it->second.dep_l.clear(); it->second.dep_arg.clear();
    }
    else
        label_table.insert({lbl, {end, {},{}}});
    return true;
}

void check_labels(std::string& err)
{
    for(auto it : label_table)
    {
        if(it.second.end == -1)
        {
            err += "Label with name " + it.first + " was never defined (semantic error)\n";
            continue;
        }
    }
}

void obj_procces(std::vector<std::vector<std::string>>& tokens, int& sctData)
{
    std::vector<std::string> labels;
    std::string err, curr_label;
    bool sec_text = false;
    bool sec_data = false;
    bool label_dcl = false;

    int i = -1;
    int j = -1; 
    int curr_address = 0;
    for(auto& line : tokens)
    {
        i++;
        if(line.empty()) continue;
        if(!sec_text && line.size() != 2)
        {
            err += LineLabel(i) + "Unexpected tokens in line (semantic error)";
            continue;
        }
        if(sec_data)
        {
            int z = 0;
            if(label_valid(line[0]))
            {
                if(label_dcl)
                {
                    err += LineLabel(i) + "Line already has label (syntatic error)\n";
                    label_dcl = false;
                    continue;
                }
                curr_label = line[0];
                label_dcl = true;
                z = 1;
            }
            if(line.size() == z) continue;
            if(line.size() > 3)
            {
                err += LineLabel(i) + "After SECTION DATA every line has at most 3 tokens (syntatic error)";
                continue;
            }
            if(!label_dcl)
            {
                err += LineLabel(i) + "Every argument in SECTION DATA has to begin with a valid label (lexical error)\n";
                continue;
            }
            if(line[z] == "SPACE") // implementar os enderecos
            {
                int m_end = 1;
                if(line.size() == z+2)
                {
                    auto [tmp_n, err_flag] = to_num(line[z+1]);
                    if(!err_flag || tmp_n <= 0)
                    {
                        err += LineLabel(i) + "Argument in SPACE has to be a valid base 10|16 number(lexical error)\n";
                        continue;
                    }
                    m_end = tmp_n;
                }
                
                ++j;
                exec_lines.push_back(std::vector<int>(m_end,0));
                curr_address += m_end;
                label_dcl = false;
                if(!push_label(curr_label, curr_address-m_end))
                {
                    err += LineLabel(i) + "Label " + curr_label + " was already defined (semantic error)\n";
                    continue;
                }
                label_table[curr_label].spaces = m_end;
            }
            else if(line[z] == "CONST") // implementar os enderecos
            {
                if(line.size() == 1 + z)
                {
                    err += LineLabel(i) + "CONST directive has to be followed by a number (syntatic error)\n";
                    continue;
                }
                auto [tmp_n, err_flag] = to_num(line[z+1]);
                
                if(!err_flag)
                {
                    err += LineLabel(i) + "Argument in CONST has to be a valid base 10|16 number(lexical error)\n";
                    continue;
                }
                if(line[z+1][0] == '\'') one_byter.insert(curr_label);
                ++j;
                exec_lines.push_back({tmp_n});
                curr_address++;
                label_dcl = false;
                if(!push_label(curr_label, curr_address-1))
                {
                    err += LineLabel(i) + "Label " + curr_label + " was already defined (semantic error)\n";
                    continue;
                }
                auto& tbl = label_table[curr_label];
                tbl.spaces = 1;
                tbl.is_const = true;
                tbl.val = tmp_n;
            }
            else
            {
                err += LineLabel(i) + "In SECTION DATA only SPACE and CONST are allowed(semantic error)\n";
                continue;
            }
        }
        else if(sec_text)
        {
            int z = 0;
            if(line.size() == 2 && line[0] == "SECTION" )
            {
                if(line[1] == "DATA")
                {
                    sctData = curr_address;
                    sec_data = true;
                }
                continue;
            }

            if(line[0][line[0].size()-1] == ':')
            {
                z = 1;
                if(!label_valid(line[0]))
                {
                    err += LineLabel(i) + "Label has to follow the following regex: ([A-Z_]+[A-Z0-9_]*:). (lexical error)\n";
                    continue;
                }
                std::string lbl_tmp = line[0].substr(0, line[0].size()-1);
        
                if(!push_label(lbl_tmp, curr_address))
                {
                    err += LineLabel(i) + "Label already defined (semantic error)\n";
                    continue;
                }
                if(label_dcl)
                {
                    err += LineLabel(i) + "Line already has label (syntatic error)\n";
                    label_dcl=false;
                    continue;
                }
                label_dcl = true;
            }
            if(line.size() == z) continue;
            if(instructions.find(line[z]) == instructions.end())
            {
                std::cout << line[z] << '\n';
                err += LineLabel(i) + " First token in line (disconsidering optiona label) has to be an instruction (syntatic error)\n";
                continue;
            }
            
            exec_lines.push_back({instructions[line[z]]});
            
            if(line[z] == "COPY" || line[z] == "INPUT_S" || line[z] == "OUTPUT_S")
            {
                int l = z + 1, r = -1;
                // bool b_flag = false;
                for(l = z + 1; line.size() > l && (r = line[l].find(',')) == std::string::npos; l++);
                if(l == line.size() || r == line[l].size()-1)
                {
                    err += LineLabel(i) + "COPY/INPUT_S/OUTPUT_S instructions have 2 arguments separated only by a comma (syntatic error)\n";
                    continue;
                }
        
                line.insert(line.begin() + l + 1, {line[l].substr(0,r), line[l].substr(r+1,line.size()-r)});
                line.erase(line.begin() + l);
               
                if(!push_arg(err, std::vector<std::string>(line.begin() + z + 1, line.begin() + l + 1),i)) continue;
                if(line[z] == "COPY")
                {
                    if(!push_arg(err, std::vector<std::string>(line.begin() + l + 1, line.end()),i)) 
                        continue;
                }
                else
                {
                    auto [t_num, pnum] = to_num(*(line.begin() + l + 1));
                    if(!pnum || t_num<0)
                    {
                        err += LineLabel(i) + "INPUT_S/OUTPUT_S second argument is a positive number (syntatic error)\n";
                        continue;
                    }
                    exec_lines[exec_lines.size()-1].push_back(t_num);
                }
                curr_address += 3;
            }
            else if(line[z] == "STOP")
            {
                if(line.size() > z + 1)
                {
                    err += LineLabel(i) + "STOP instruction has no arguments(syntatic error)\n";
                    continue;
                }
                
                curr_address++;
            }
            else
            {
                if(line.size() > z + 4 || line.size() == z+3 || line.size() == z+1)
                {
                    err += LineLabel(i) + "Unexpected number of arguments in line (syntatic error)\n";
                    continue;
                }
                if(!var_valid(line[z + 1]))
                {
                    err += LineLabel(i) + "Invalid Label Format (lexical error)\n";
                    continue;
                }
                if(!push_arg(err, std::vector<std::string>(line.begin() + z + 1, line.end()),i)) continue;
                curr_address += 2;
            }
            label_dcl = false;
        }
        else if(line.size() == 2 && line[0] == "SECTION" )
        {
            if(line[1] == "TEXT")
                sec_text = true;
            else if(line[1] == "DATA")
            {
                sec_data = true;
                sctData = curr_address;
                if(!sec_text)
                {
                    err += LineLabel(i) + "SECTION DATA declared before SECTION TEXT (semantic error)\n";
                    continue;
                }
            }
        }
        else
        {
            err += LineLabel(i) + "Unexpected tokens (semantic error)\n";
            continue;
        }
    }
    if(!sec_text)
        err += "SECTION TEXT not declared (semantic error)\n";
    check_labels(err);
    if(err.size())
    {
        std::cout << "compilation dropped due to errors:\n\n" << err << std::endl;
        exit(EXIT_FAILURE);
    }
}

void pre_proccess_file(std::vector<std::vector<std::string>>& tokens, std::string ftype = "bin.PRE")
{
    std::ofstream fout(ftype);

    for(auto& line : tokens)
    {
        if(line.empty()) continue;

        for(auto& token : line)
            fout << token << ' ';
        fout << '\n';
    }
    fout.close();
}

void debug_file(std::vector<std::vector<std::string>>& tokens, std::string ftype = ".DBG")
{
    std::ofstream fout(std::string("./bin") + ftype);

    for(auto& line : tokens)
    {
        for(auto& token : line)
            fout << token << ' ';
        fout << '\n';
    }
    fout.close();
}

void push_macro_tokens(
    macro_info& mc_info,
    int tk_info_idx,
    std::vector<std::vector<std::string>>& mcr_tokens, 
    std::vector<std::vector<std::string>>& rcv,
    int init_pos = -1
    )
{
    if(mcr_tokens.empty()) return;
    
    std::vector<std::string>& tk_info = mc_info.tokens[tk_info_idx];
    
    std::vector<std::vector<std::string>> tmp_test;
    for(auto& line : mcr_tokens)
    {
        if(line.empty()) continue;
        
        tmp_test.push_back({});
        
        std::vector<std::string>& rcv_temp = *(tmp_test.end()-1);
        
        for(auto token : line)
        {
            int j = 0;
            
            for(auto& jt : mc_info.mapped)
            {
                int pos = -1; 
                
                while((pos = token.find(jt, pos+1)) != std::string::npos)
                    token.replace(pos, jt.size(), tk_info[j]);

                j++;
            }
            rcv_temp.push_back(token);
        }
    }
    rcv.insert(rcv.begin() + init_pos + 1, tmp_test.begin(), tmp_test.end());
}

void macro_unpack(
    std::vector<std::vector<std::string>>& tokens, 
    std::map<std::string, macro_info>& macros,
    std::map<std::string, std::vector<std::vector<std::string>>>& macro_unpk,
    std::map<int, std::map<std::string, macro_info>::iterator>& calls
    )
{
    std::map<int, std::map<std::string, macro_info>::iterator> places;
    for(auto it = macros.begin(); it != macros.end(); it++)
    {;
        places.insert({it->second.l, it});
        for(auto& jt : it->second.tokens)
            calls.insert({jt.first, it});
    }
    
    for(auto it = places.begin(), jt = calls.begin(); it!=places.end(); it++)
    {
        auto zt = macro_unpk.insert({it->second->first, {}}).first;
        
        tokens[it->second->second.l].clear();

       
        for(int i = it->second->second.l+1; i < it->second->second.r; i++)
        {
            if(jt != calls.end() && jt->first == i)
                push_macro_tokens(jt->second->second, i, macro_unpk[jt->second->first], zt->second);
            else
            {
                zt->second.push_back(tokens[i]);
                tokens[i].clear(); 
            }
            if(jt != calls.end() && jt->first <= i)
            {
                jt = calls.erase(jt);
            }
        }
        tokens[it->second->second.r].clear();
    }
}

void macro_process_tokens(std::vector<std::vector<std::string>>& tokens, std::map<std::string, macro_info>& macros)
{
    std::map<std::string, std::vector<std::vector<std::string>>> macro_unpk;
    std::map<int, std::map<std::string, macro_info>::iterator> calls;

    macro_unpack(tokens, macros, macro_unpk, calls);
    
    for(auto it = calls.begin(); it != calls.end();it++)
        push_macro_tokens(it->second->second, it->first, macro_unpk[it->second->first], tokens, it->first);
}

void push_obj(std::string fname)
{
    std::ofstream fout(fname + ".OBJ");
    for(auto& line : exec_lines)
        for(auto& end : line)
            fout << end << ' ';
    fout.close();
}

int label_add_num(std::map<int, std::string>& label_ins, int nmbr)
{
    int plus = 0;
    for(std::map<int,std::string>::iterator it = label_ins.begin(), jt = std::next(it);; it++, jt++)
    {
        if(it->first == nmbr)
        {
            plus = 0; break;
        }
        else if(jt == label_ins.end() || (it->first < nmbr && nmbr < jt->first))
        {
            plus = nmbr - it->first; break;
        }   
    }
    return plus;
}

void traduct(std::string file_n, int sctData)
{
    int end = 0;
    std::ofstream fout(file_n);

    std::map<int, std::string> label_ins;
    int plus, plus1, plus2;
    for(auto& it : label_table)
        label_ins.insert({it.second.end, it.first});
    {
        std::ifstream fin("functions.asm");
        char buff[512];
        while(!fin.eof())
        {
            fin.getline(buff,511);
            fout << buff << '\n';
        }
        fin.close();
    }
    for(auto it : one_byter)
        std::cout << it;
    std::cout << '\n';
    fout << "\nsection .text\nglobal _start\n_start:\n";
    for(int i = 0; end < sctData; i++)
    {
        std::string line;
        std::string reg = "eax";
        auto it = label_ins.find(end);
        if(it != label_ins.end())
            line += it->second + ": ";
        
        switch(exec_lines[i][0])
        {
            case 1 ... 4:
                plus = label_add_num(label_ins, exec_lines[i][1]);
                if(exec_lines[i][0] >= 3)
                    line += "cdq\n";
                line += instructions_32[exec_lines[i][0]];
                if(one_byter.find(label_ins[exec_lines[i][1] - plus]) != one_byter.end())
                    reg = "al";
                
                if(exec_lines[i][0] < 3)
                    line += " " + reg + ",";
                if(reg[0] == 'e')
                    line += " dword ";
                line += " [" + label_ins[exec_lines[i][1] - plus];
               
                if(plus != 0)
                    line += " + " + std::to_string(plus);
                line += "]\n";
            break;

            case 5 ... 8:
                if(exec_lines[i][0] != 5)
                    line += "cmp eax, 0\n";
                line += instructions_32[exec_lines[i][0]] + ' ' + label_ins[exec_lines[i][1]] + '\n';
            break;

            case 9:
                plus1 = label_add_num(label_ins, exec_lines[i][1]);
                plus2 = label_add_num(label_ins, exec_lines[i][2]);
                reg = "ebx";
                if(one_byter.find(label_ins[exec_lines[i][1] - plus]) != one_byter.end())
                    reg = "bl";
                line += "mov " + reg + ",";
                
                line += "[" + label_ins[exec_lines[i][1] - plus1];
                if(plus1 != 0)
                    line += " + " + std::to_string(plus1);
                line += "]\n";
                line += "mov [" + label_ins[exec_lines[i][2] - plus2];
                if(plus2 != 0)
                    line += " + " + std::to_string(plus2);
                line += "], " + reg + '\n';
            break;

            case 10:
                plus = label_add_num(label_ins, exec_lines[i][1]);
                if(one_byter.find(label_ins[exec_lines[i][1] - plus]) != one_byter.end())
                    reg = "al";
                line += "mov " + reg + ", [" + label_ins[exec_lines[i][1] - plus];
                if(plus != 0)
                    line += " + " + std::to_string(plus);
                line += "]\n";
            break;

            case 11:
                plus = label_add_num(label_ins, exec_lines[i][1]);
                if(one_byter.find(label_ins[exec_lines[i][1] - plus]) != one_byter.end())
                    reg = "al";
                line += "mov [" + label_ins[exec_lines[i][1] - plus];
                if(plus != 0)
                    line += " + " + std::to_string(plus);
                line += "], " + reg + '\n';
            break;

            case 14: // stop
                line += "mov eax, 1\nmov ebx, 0\nint 80h\n";
            break;

            case 12 ... 13: case 15 ... 18: // input_c
                plus = label_add_num(label_ins, exec_lines[i][1]);
                line += "push " + label_ins[exec_lines[i][1] - plus];
                if(plus != 0)
                    line += " + " + std::to_string(plus);
                line += '\n';
                if(exec_lines[i][0] == 17 || exec_lines[i][0] == 19)
                    line += "push dword " + std::to_string(exec_lines[i][2]) + '\n';
                line += "call " + instructions_32[exec_lines[i][0]] + '\n';
            break;
        }
        fout << line;
        end += exec_lines[i].size();
    }
    fout << "\nsection .data\n";
    for(auto& it : label_table)
        if(it.second.spaces != 0 && it.second.is_const)
        {
            fout << it.first;
            if(one_byter.find(it.first) != one_byter.end())
                fout << " db "; 
            else
                fout << " dd ";
            fout << std::to_string(it.second.val) << '\n';
        }
    fout << "\nsection .bss\n";
    for(auto& it : label_table)
        if(it.second.spaces != 0 && !it.second.is_const)
            fout << it.first << " resd " << std::to_string(it.second.spaces) << '\n';
        
    fout.close();
}

int main(int argc, char **argv)
{
    std::vector<std::vector<std::string>> tokens;
    std::map<std::string, macro_info> macros;
    int sctData = -1;
    std::string file_n = argv[1];

    read_asm(tokens, (file_n + ".ASM").c_str());
    upper_case(tokens);
    pre_proccess(tokens);
    
    pre_proccess_file(tokens, file_n + ".PRE");

    macro_processing(tokens, macros);
    macro_process_tokens(tokens, macros);
    pre_proccess_file(tokens, file_n + ".MCR");

    obj_procces(tokens,sctData);
    traduct(file_n + ".S", sctData);
}