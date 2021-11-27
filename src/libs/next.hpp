#ifndef NEXT_HPP
#define NEXT_HPP

#include <string>
#include <sstream>

inline std::string next(std::string & str){
    auto div = str.find_first_of(" \t");
    decltype(div) txt;
    
    if(div == 0){
        txt = str.find_first_not_of(" \t");
        if(txt == std::string::npos)
            txt = str.size();
        str = str.substr(txt);
        div = str.find_first_of(" \t");
    }
    
    div = std::min(div, str.size());
    std::string ret = std::string(str.begin(), str.begin() + std::ptrdiff_t(div));
    
    
    if(div == str.size()){
        str = "";
    }
    else{
        str.erase(0, div + 1);
    }
    
    return ret;
}

inline std::string peek(std::string & str){
    auto div = str.find_first_of(" \t");
    decltype(div) txt;
    
    if(div == 0){
        txt = str.find_first_not_of(" \t");
        if(txt == std::string::npos)
            txt = 0;
        str = str.substr(txt);
        div = str.find_first_of(" \t");
    }
    
    div = std::min(div, str.size());
    std::string ret = std::string(str.begin(), str.begin() + std::ptrdiff_t(div));
    
    return ret;
}

template <typename data_type> 
inline data_type next_as(std::string & str){
    data_type data = data_type();
    std::string arg = next(str);
    if(arg != ""){
        std::istringstream(arg) >> data;
    }
    return data;
}

template <typename data_type> 
inline data_type next_as(std::string & str, data_type fail){
    data_type data;
    std::string arg = next(str);
    if(arg != "" && arg != "default" && arg != "_"){
        std::istringstream(arg) >> data;
    }
    else{
        data = fail;
    }
    return data;
}

template <>
inline bool next_as<bool>(std::string & str, bool fail){
    std::string arg = next(str);
    if(arg == "true" || arg == "1"){
        return true;
    }
    else if(arg == "false" || arg == "0"){
        return false;
    }
    else{
        return fail;
    }
}

// returns true if successful
inline bool apply_bool(std::string input, bool & output, bool if_empty = false){
    std::string arg = next(input);
    if(arg.size() == 0){
        output = if_empty;
        return true;
    }
    else if(arg == "true"){
        output = true;
        return true;
    }
    else if(arg == "false"){
        output = false;
        return true;
    }
    else{
        return false;
    }
}

#endif // NEXT_HPP
