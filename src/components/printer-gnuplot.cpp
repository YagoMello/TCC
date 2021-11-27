// INCLUDE <>
#include <iostream>
#include <thread>

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

static constexpr const char * gnuplot_printer_help_string = 
R"---(printer::gnuplot V1.0 2021-09-05

|------------------|----------------|
|     argument     |      type      |
|==================|================|
| sampling mode    | string         |
| interval/points  | double/size_t  |
| print mode       | string         |
| time to start    | double         |
| precision        | double         |
|------------------|----------------|

sampling mode:
  interval: fixed interval defined by the user
  points:   fixed number of points dependent on duration

interval/points:
  The value for the sampling mode

print mode:
  none:     The last value of the interval.
  average:  The average value of the interval.
  max:      The largest value.                         -100  <  10  =>   10
  min:      The smallest value.                          -1  <  10  =>   -1
  max-abs:  The largest absolute value. Keeps sign.   |-100| > |10| => -100
  min-abs:  The smallest absolute value. Keeps sign.  | -10| > |-1| =>   -1
  max-ampl: Maximum value of the amplitude. Positive. |-100| > |10| =>  100
  min-ampl: Minimum value of the amplitude. Positive. | -10| > |-1| =>    1

time to start:
  The time when the component starts recording data

precision:
  Number of digits of text when converting.

When using custom types, make a custom "to_string" with
the same signature of std::to_string to be able to print the value.
To be able to use a custom type with print-mode "average", make a
conversion operator from the type to double.
)---";

class gnuplot_printer_t : public comp::component_t {
public:
    ~gnuplot_printer_t() override {
        close_file();
        free_resources();
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
        
        if(!mfind(print::list::names, next(args), print_mode_)){
            info << "[ERROR] Print mode unkonwn!\n";
            return info;
        }
        
        time_start_print_ = next_as<double>(args, 0);
        precision_ = next_as<uint16_t>(args, 10);
        
        return info;
    }
    
    void setup(sim::simulator_t * simulator, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        if(print_points_ != 0){
            print_interval_ = (simulator->duration() - time_start_print_) / (double(print_points_) - 0.5);
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
        free_resources();
        time_next_print_ = time_start_print_;
        run_gnuplot_setup_ = 3;
    }
    
    void update(double /*time*/, double /*last_time_step*/) override { }
    
    void printer(const double time, const double time_step, const uint64_t iteration) override {
        data_filter_.apply_iteration(time_step);
        
        if(time >= time_next_print_){
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
            
            if(run_gnuplot_setup_ != 0){
                if(run_gnuplot_setup_ == 1){
                    gnuprint();
                }
                run_gnuplot_setup_--;
            }
            
        }
    }
    
    bool keep_simulation_alive(const double /*time*/) override {
        return true;
    }
    
    double time_step_max(const double time) const override {
        return time_next_print_ - time;
    }
    
    void simulation_complete() override {
        if(print_thread_status_ == RUN){
            print_thread_status_ = UPDATE_AND_STOP;
        }
        while(print_thread_status_ != STOP);
        close_file();
    }
    
    const char * help() const override {
        return gnuplot_printer_help_string;
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
    
    enum thread_status : uint8_t {
        STOP,
        RUN,
        UPDATE_AND_STOP
    };
    
    static void gnuplotupdate(FILE * data, FILE * pipe, uint8_t * status){
        while(*status != STOP){
            using namespace std::chrono_literals;
            
            fflush(data);
            fprintf(pipe, "\nreplot\n");
            fflush(pipe);
            
            if(*status == UPDATE_AND_STOP)
                *status = STOP;
            
            std::this_thread::sleep_for(500ms);
        }
    }
    
    void free_resources(){
        if(print_thread_status_ == RUN){
            print_thread_status_ = UPDATE_AND_STOP;
            while(print_thread_status_ != STOP);
        }
        if(print_thread_.joinable()){
            print_thread_.join();
        }
        if(gnuplotpipe_){
            pclose(gnuplotpipe_);
            gnuplotpipe_ = nullptr;
        }
    }
    
    static size_t count_hidden_nodes(const std::vector<printable::var_data_t> & node_vector){
        size_t count = 0;
        for(const auto & node : node_vector){
            if(node.is_active){
                count++;
            }
        }
        return count;
    }
    
    void gnuprint(){
        size_t count = count_hidden_nodes(data_filter_.nodes());
        
        gnuplotpipe_ = popen("gnuplot -d", "w");
        fprintf(gnuplotpipe_, "set datafile separator ','\n");
        fprintf(gnuplotpipe_, "set ylabel \"Voltage [V]\"\n");
        if(data_filter_.variables().size() > 0){
            fprintf(gnuplotpipe_, "set ytics nomirror\n");
            fprintf(gnuplotpipe_, "set y2tics\n");
            fprintf(gnuplotpipe_, "set y2label \"Component Data\"\n");
        }
        fprintf(gnuplotpipe_, "set xlabel \"Time [s]\"\n");
        fprintf(gnuplotpipe_, "set key autotitle columnhead\n");
        fprintf(gnuplotpipe_, "plot for [col=3:%lu] '%s' using 1:col axes x1y1 with lines linewidth 1.25,", count + 2, file_name_.c_str());
        if(data_filter_.variables().size() > 0){
            fprintf(gnuplotpipe_, " for [col=%lu:%lu] '' using 1:col axes x1y2 with lines linewidth 1.25 dashtype 4", count + 3, count + 2 + data_filter_.variables().size());
        }
        //fprintf(gnuplotpipe_,      "for [col=3:%lu] '' using 1:col with points pointtype 7 \n", count + 2);
        //points pointtype 1 pointsize 0.25
        print_thread_status_ = RUN;
        print_thread_ = std::thread(gnuplotupdate, file_, gnuplotpipe_, &print_thread_status_);
    }
    
    uint16_t precision_;
    
    data_filter_t data_filter_;
    print::mode print_mode_ = print::mode::NONE;
    
    uint8_t run_gnuplot_setup_ = 3;
    FILE * gnuplotpipe_ = nullptr;
    
    std::thread print_thread_;
    uint8_t print_thread_status_ = STOP;
    
    double time_start_print_;
    double time_next_print_;
    double print_interval_;
    size_t print_points_;
    
    std::string file_name_ = "temp.csv";
    FILE * file_ = nullptr;
};

autoreg::reg<gnuplot_printer_t> gnuplot_printer_reg(database::components(), "printer::gnuplot");
