#ifndef MFIND_HPP
#define MFIND_HPP

#include <map>

template <typename T1, typename T2>
T2 * mfind(std::map<T1, T2> & imap, const auto & key) {
    auto iter = imap.find(key); // iter type is "typename std::map<T1, T2>::iterator"
    if(iter != imap.end())
        return &iter->second;
    else
        return nullptr;
}

bool mfind(const auto & imap, const auto & key, auto & value){
    auto node = imap.find(key);
    bool found = (node != imap.end());
    if(found) value = node->second;
    return found;
}

/*
auto mfind(auto & imap, const auto & key) -> decltype(imap.find(key)->second) * {
    auto node = imap.find(key);
    if(node != imap.end()) 
        return &node->second; 
    else
        return nullptr;
}

bool msearch(const auto & imap, const auto & value, auto & key){
    auto node = find_if(imap.begin(), imap.end(), [value](auto & p) {return p.second == value;});
    bool found = (node != imap.end());
    if(found) key = node->first;
    return found;
}
*/
#endif // MFIND_HPP
