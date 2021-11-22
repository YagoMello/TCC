#include "cli.hpp"

#include <ctime>
#include <iomanip>

void cli_t::warn(const std::string & what){
    std::time_t time_utc = time(nullptr);
    std::tm time_local = *std::localtime(&time_utc);
    
    logs_.warnings << std::put_time(&time_local, "%F %T ") << what << "\n";
    logs_.all << std::put_time(&time_local, "%F %T ") << "[WARNING] " << what << "\n";
    
    std::cout << what << "\n";
}

void cli_t::error(const std::string & what){
    std::time_t time_utc = time(nullptr);
    std::tm time_local = *std::localtime(&time_utc);
    
    logs_.errors << std::put_time(&time_local, "%F %T ") << what << "\n";
    logs_.all << std::put_time(&time_local, "%F %T ") << "[ERROR] " << what << "\n";
    
    std::cout << what << "\n";
}

void cli_t::log(const std::string & what){
    std::time_t time_utc = time(nullptr);
    std::tm time_local = *std::localtime(&time_utc);
    
    logs_.all << std::put_time(&time_local, "%F %T ") << "[INFO] " << what << "\n";
}
