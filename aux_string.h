#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <functional>
#include <map>
#include <regex>

template <class T>
bool is_any(std::vector<T>& rng, T cmp_val, std::function<bool(T,T)> cmp_fnc)
{
    for(auto& t_val : rng)
        if(cmp_fnc(t_val, cmp_val))
            return true;
    return false;
}

//sep are the tokens like ' ', which separate tokens but arent tokens themselves.
//spec are the tokens like ':' which separete 2 tokens, and are tokens themselves. They are appended into whatever token was being written before. Ex: "label:"
void tokenize(
        std::vector<std::vector<std::string>>& tkns,
        std::string input, 
        std::vector<char>& sep, 
        std::vector<char>& spec
    )
{
    tkns.push_back(std::vector<std::string>());
    std::vector<std::string>& tokens = tkns[tkns.size()-1];
    bool new_token = true;
    int lToken = 0;
    
    auto sepcomp = std::function<bool(char,char)>([](char vec_inp, char inp) -> bool { return vec_inp == inp; });    

    for(int i = 0; i < input.size(); i++)
    {
        if(input[i]==';')
            break;
        
        else if(is_any(spec, input[i], sepcomp))
        {
            if(new_token) lToken=i;

            tokens.push_back(input.substr(lToken, i-lToken+1));
            new_token = true;
        }

        else if(is_any(sep, input[i], sepcomp))
        {
            if(new_token) continue;
            tokens.push_back(input.substr(lToken, i - lToken));
            new_token = true;
        }

        else if(new_token)
        {
            new_token = false;
            lToken = i;
        }
        // std::cout << i << '\n';
    }

    if(!new_token) tokens.push_back(input.substr(lToken, input.size()-lToken));
    // return tokens;
}

void upper_case(std::vector<std::vector<std::string>>& tokens)
{
    for(auto& line : tokens)
        for(auto& token : line)
            for(auto& ch : token)
                if(ch >= 'a' && ch <= 'z') ch += 'A' - 'a';
}

std::string LineLabel(int i)
{ return "Line (" + std::to_string(i+1) + ") : "; }

bool to_num(std::string& number)
{
    short int shr;
    bool err = false;
    if(number.size() <= 1 || number[1] != 'X')
    {
        try
        { shr = std::stoi(number); }
        catch(...)
        { err = true; }
    }
    else
    {
        try{ shr = std::stoi(number, nullptr, 16);}
        catch(...) { err = true;}
    }
    return err;
}

bool label_valid(std::string& label)
{ return std::regex_search(label, std::regex("[A-Z_]+[A-Z0-9_]*:")); }

bool var_valid(std::string& label)
{ return std::regex_search(label, std::regex("[A-Z_]+[A-Z0-9_]*")); }

bool macro_arg(std::string& label)
{ return std::regex_search(label, std::regex("&[A-Z_]+[A-Z0-9_]*[,]?")); }

bool call_arg(std::string& label)
{ return std::regex_search(label, std::regex("[A-Z_]+[A-Z0-9_]*[,]?")); }