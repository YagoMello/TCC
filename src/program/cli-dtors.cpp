#include "cli.hpp"

#include <ctime>
#include <iomanip>
#include <fstream>
#include <filesystem>

cli_t::~cli_t(){
    log("<~cli_t> entering destructor");
    
    if(!std::filesystem::exists("logs")){
        log("<~cli_t> directory \"logs\" doesn't exist");
        log("<~cli_t> creating directory \"logs\"");
        std::filesystem::create_directory("logs");
    }
    else{
        log("<~cli_t> found directory \"logs\"");
    }
    
    log("<~cli_t> reading local time");
    std::time_t time_utc = time(nullptr);
    std::tm time_local = *std::localtime(&time_utc);
    
    std::stringstream file_name_warning;
    std::stringstream file_name_error;
    std::stringstream file_name_log;
    
    log("<~cli_t> creating file names");
    file_name_warning << "logs/" << std::put_time(&time_local, "%F_%H_%M_%S") << "-warnings.txt";
    file_name_error << "logs/" << std::put_time(&time_local, "%F_%H_%M_%S") << "-errors.txt";
    file_name_log << "logs/" << std::put_time(&time_local, "%F_%H_%M_%S") << "-log.txt";
    
    log("<~cli_t> creating files");
    std::ofstream file_warning(file_name_warning.str());
    std::ofstream file_error(file_name_error.str());
    std::ofstream file_log(file_name_log.str());
    
    log("<~cli_t> writing data");
    file_warning << logs_.warnings.rdbuf();
    file_error << logs_.errors.rdbuf();
    file_log << logs_.all.rdbuf();
    
    log("<~cli_t> done. goodbye");
}

