#ifndef UNDO_HPP
#define UNDO_HPP

#include <fstream>
#include <cstdlib>
#include <string>
#include <list>

namespace action_log{

class logger_t {
public:
    logger_t(const std::string & file_name = "");
    std::string undo();
    std::string redo();
    void add_undo(const std::string & action);
    void add_redo(const std::string & action);
    void clear();
    void save_to_file(const std::string & file_name);
    void read_from_file(const std::string & file_name);
private:
    std::list<std::string> log_undo_;
    std::list<std::string> log_redo_;
};

inline logger_t::logger_t(const std::string & file_name){
    if(file_name != ""){
        logger_t::read_from_file(file_name);
    }
}

inline std::string logger_t::undo(){
    std::string command;
    
    if(!log_undo_.empty()){
        command = log_undo_.back();
        log_undo_.pop_back();
    }
    
    return command;
}

inline std::string logger_t::redo(){
    std::string command;
    
    if(!log_redo_.empty()){
        command = log_redo_.back();
        log_redo_.pop_back();
    }
    
    return command;
}

inline void logger_t::add_undo(const std::string & action){
    if(action != ""){
        log_undo_.push_back(action);
    }
}

inline void logger_t::add_redo(const std::string & action){
    if(action != ""){
        log_redo_.push_back(action);
    }
}

inline void logger_t::clear(){
    log_undo_.clear();
    log_redo_.clear();
}

inline void logger_t::save_to_file(const std::string & file_name){
    std::ofstream file_undo(file_name + "-undo.txt");
    std::ofstream file_redo(file_name + "-redo.txt");
    
    for(const std::string & command : log_undo_){
        file_undo << command << "\n";
    }
    for(const std::string & command : log_redo_){
        file_redo << command << "\n";
    }
}

inline void logger_t::read_from_file(const std::string & file_name){
    std::ifstream file_undo(file_name + "-undo.txt");
    std::ifstream file_redo(file_name + "-redo.txt");
    
    logger_t::clear();
    
    std::string command;
    while(std::getline(file_undo, command)){
        log_undo_.emplace_back(command);
    }
    while(std::getline(file_redo, command)){
        log_redo_.emplace_back(command);
    }
}

}

#endif // UNDO_HPP
