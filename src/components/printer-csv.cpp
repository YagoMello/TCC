// INCLUDE <>
#include <iostream>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../simulator/data-filter.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../libs/mfind.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"
#include "../libs/shared_var.hpp"

static constexpr const char * csv_printer_help_string = 
R"---(printer::csv V1.0 2021-09-05

|------------------|----------------|
|     argument     |      type      |
|==================|================|
| sampling mode    | string         |
| interval/points  | double/size_t  |
| file name        | string         |
| print mode       | string         |
| precision        | double         |
| dump             | bool           |
|------------------|----------------|

sampling mode:
  interval: fixed interval defined by the user
  points:   fixed number of points dependent on duration

interval/points:
  The value for the sampling mode

file name:
  The data will be witten to "file-name.csv".
  If the file name is "stdout", the output will be redirected to the terminal.
  If the file name is empty, a file new file will be created and named as the
  label of the component + ".csv".

print mode:
  none:     The last value of the interval.
  average:  The average value of the interval.
  max:      The largest value.                         -100  <  10  =>   10
  min:      The smallest value.                          -1  <  10  =>   -1
  max-abs:  The largest absolute value. Keeps sign.   |-100| > |10| => -100
  min-abs:  The smallest absolute value. Keeps sign.  | -10| > |-1| =>   -1
  max-ampl: Maximum value of the amplitude. Positive. |-100| > |10| =>  100
  min-ampl: Minimum value of the amplitude. Positive. | -10| > |-1| =>    1

precision:
  Number of digits of text when converting.

dump:
  Accepts "true" or "false". If true will print every iteration of the simulator.
  Using this mode creates enormous files. Use with stdout or piping to other program.

When using custom types, make a custom "to_string" with
the same signature of std::to_string to be able to print the value.
To be able to use a custom type with print-mode "average", make a
conversion operator from the type to double.
)---";

class csv_printer_t : public comp::component_t {
public:
    ~csv_printer_t() override {
        close_file();
    }
    
    std::stringstream configure(node_map_type & /*nmap*/, printable_map_type & /*pmap*/, shared::list & /*ls*/, std::string args) override {
        std::stringstream info;
        
        const std::string sampling_mode = next(args);
        
        if(sampling_mode == "interval"){
            print_interval_ = next_as<double>(args, 10e-3);
            print_points_   = 0;
        }
        else if(sampling_mode == "points"){
            print_interval_ = 0;
            print_points_   = next_as<size_t>(args, 1000);
        }
        else {
            info << "[ERROR] Sampling mode unknown. Available options are \"interval\" and \"points\"\n";
            return info;
        }
        
        std::string file_name_candidate = next(args);
        
        if(!mfind(print::list::names, next(args), print_mode_)){
            info << "[ERROR] Print mode unkonwn!\n";
            return info;
        }
        
        precision_ = next_as<uint16_t>(args, 10);
        dump_ = next_as<bool>(args, false); 
        
        if(file_name_candidate.empty()){
            file_name_ = this->label() + ".csv";
        }
        else{
            file_name_ = file_name_candidate;
        }
        
        return info;
    }
    
    void setup(sim::simulator_t * simulator, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        if(print_points_ != 0){
            print_interval_ = simulator->duration() / (double(print_points_) - 0.5);
        }
    }
    
    void setup_printer(const std::vector<node::node_t *> & node_vector, const std::vector<printable::var_info_t *> & variable_vector) override {
        if(!file_) open_file();
        
        data_filter_.clear();
        data_filter_.assign_variables(node_vector, variable_vector);
        data_filter_.print_mode(print_mode_);
        data_filter_.precision(precision_);
        
        fprintf(file_, "time,iteration");
        
        for(const auto * node : node_vector){
            const std::string & label = node->label();
            
            if(!node->is_hidden()){
                if(label != ""){
                    fprintf(file_, ",%s", label.c_str());
                }
                else{
                    fprintf(file_, ",undef");
                }
            }
        }
        
        for(const auto * info : variable_vector){
            fprintf(file_, ",%s [%s]", info->name.c_str(), info->unit.c_str());
        }
        
        fprintf(file_, "\n");
    }
    
    void clear_variables() override {
        time_next_print_ = 0;
    }
    
    void update(double /*time*/, double /*last_time_step*/) override { }
    
    void printer(const double time, const double time_step, const uint64_t iteration) override {
        data_filter_.apply_iteration(time_step);
        
        if(time >= time_next_print_ || dump_){
            fprintf(file_, "%e,%lu", time, iteration);
            
            for(auto & node_data : data_filter_.nodes()){
                if(node_data.is_active){
                    fprintf(file_, ",%s", node_data.printer(node_data).c_str());
                    node_data.clear_print_memory(node_data);
                }
            }
            
            for(auto & var_data : data_filter_.variables()){
                if(var_data.is_active){
                    fprintf(file_, ",%s", var_data.printer(var_data).c_str());
                    var_data.clear_print_memory(var_data);
                }
            }
            
            data_filter_.clear_print_memory();
            
            fprintf(file_, "\n");
            time_next_print_ += print_interval_;
        }
    }
    
    bool keep_simulation_alive(const double /*time*/) override {
        return true;
    }
    
    double time_step_max(const double time) const override {
        return time_next_print_ - time;
    }
    
    void simulation_complete() override {
        close_file();
    }
    
    const char * help() const override {
        return csv_printer_help_string;
    }
    
private:
    void open_file(){
        if(file_name_ == "stdout")
            file_ = stdout;
        else
            file_ = fopen(file_name_.c_str(), "w");
        
        if(file_ == nullptr){
            perror("fopen() failed!");
            exit(0);
        }
    }
    
    void close_file(){
        if(file_ != nullptr){
            fclose(file_);
            file_ = nullptr;
        }
    }
    
    uint16_t precision_;
    
    data_filter_t data_filter_;
    print::mode print_mode_ = print::mode::NONE;
    
    bool dump_ = false;
    
    double time_next_print_;
    double print_interval_;
    size_t print_points_;
    
    std::string file_name_ = "unnamed.csv";
    FILE * file_ = nullptr;
};

autoreg::reg<csv_printer_t> csv_printer_reg(database::components(), "printer::csv");
