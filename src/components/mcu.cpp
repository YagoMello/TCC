#include <map>
#include <cinttypes>
#include <cstdio>
#include <climits>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <memory>
#include <limits>
#include <regex>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../libs/mfind.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

class mcu_gpio_driver_t : public comp::component_t {
public:
    enum status : uint8_t {
        LOW,
        HIGH,
        HI_Z
    };
    std::stringstream configure(node_map_type &, printable_map_type &, shared::list &, std::string) override {
        std::stringstream info;
        return info;
    }
    
    void clear_variables() override {
        current_ = 0;
        status_  = HI_Z;
    }
    
    void update(double, double) override {
        const double vpo = voltage_difference(node_positive_, node_output_);
        const double vno = voltage_difference(node_negative_, node_output_);
        std::cout << "vpo/vno: [" << vpo << "," << vno << "] - ";
        switch(status_){
            case LOW:
                std::cout << "LOW";
                current_ = vno / resistance_;
                node_negative_->current_out(current_);
                node_output_->current_in(current_);
                break;
            case HIGH:
                std::cout << "HIGH";
                current_ = vpo / resistance_;
                node_positive_->current_out(current_);
                node_output_->current_in(current_);
                break;
            case HI_Z:
                std::cout << "HI_Z";
                break;
            default:
                throw;
        }
        std::cout << "\n";
        
    }
    
    void resistance(double value) {
        resistance_ = value;
    }
    
    [[nodiscard]] double current() const {
        return current_;
    }
    
    void set_nodes(node::node_t * pos, node::node_t * neg, node::node_t * out){
        node_positive_ = pos;
        node_negative_ = neg;
        node_output_   = out;
    }
    
    void status(uint8_t value) {
        status_ = value;
    }
    
private:
    // sim var
    double current_ = 0;
    uint8_t status_ = HI_Z;
    
    //params
    double resistance_  = 1000;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
    node::node_t * node_output_;
};

/*
 * Author:  Yago T. de Mello
 * Version: 0.6.1
 * Date:    2021-09-12
 * License: MIT
 */

/*
 * Every instruction uses 4 bytes:
 * opcode arg1 arg2 arg3
 * 
 * Disclaimer:
 * [argX] represents the valued stored at argX
 * 
 * The instructions are:
 * NOP:     No operation
 * MOVL:    Move Literal,   move the litreral arg2  to [arg1]
 * MOVF:    Move File,      move             [arg2] to [arg1]
 * CMOVL:   Conditional Move Literal,       move the litreral arg2  to [arg1] if [arg3] != 0
 * CMOVF:   Conditional Move File,          move             [arg2] to [arg1] if [arg3] != 0
 * CNMOVL:  Conditional Not Move Literal,   move the litreral arg2  to [arg1] if [arg3] == 0
 * CNMOVF:  Conditional Not Move File,      move             [arg2] to [arg1] if [arg3] == 0
 * RET:     Return,         returns from a call, popping the stack
 * ADDL:    Add Literal,    add the literal  arg2  to [arg1] and insert the result in [arg3]
 * ADDF:    Add File,       add the literal [arg2] to [arg1] and insert the result in [arg3]
 * SUBL
 * SUBF
 * MULL
 * MULF
 * DIVL
 * DIVF
 * MODL     Modulo Literal, [arg1] = [arg2] %  arg3
 * MODF     Module File,    [arg1] = [arg2] % [arg3]        
 * INC:     Increment,      add 1 to [arg1]
 * DEC
 * AJ       Absolute Jump,  Jump inconditionally to the literal address arg1
 * AJGTL    Absolute jump if greater than literal, jump to [arg1] if [arg2] > literal arg3
 * AJGTF    Absolute jump if greater than file,    jump to [arg1] if [arg2] > [arg3]
 * AJEQL    --               equal        literal
 * AJEQF    --               equal        file
 * AJLTL    --               less than    literal
 * AJLTF    --               less than    file
 * AJGTEL   --               greater than or equal literal
 * AJGTEF   --               greater than or equal file
 * AJLTEL   --               less than or equal    literal
 * AJLTEF   --               less than or equal    file
 * ACALL    Absolute Call,  stores the PC in the stack and jumps to the literal address arg1
 * RJ       Relative Jump,  Jump inconditionally to PC + arg1
 * RJGTL
 * RJGTF
 * RJEQL
 * RJEQF
 * RJLTL
 * RJLTF
 * RJGTEL
 * RJGTEF
 * RJLTEL
 * RJLTEF
 * RCALL
 * RRL      Rotate Right Literal, Rotate [arg2] literal arg3 times and insert the result in [arg1]      [arg1] = [arg2] >>  arg3 
 * RRF      Rotate Right File,    Rotate [arg2] [arg3] times and insert the result in [arg1]            [arg1] = [arg2] >> [arg3]
 * RLL
 * RLF
 * BORL     Binary Or Literal,    [arg1] = [arg2] |  arg3
 * BORF     Binary Or File,       [arg1] = [arg2] | [arg3]
 * BANDL
 * BANDF
 * BXORL
 * BXORF
 * BNOT     Binary not,           [arg1] = ~[arg2]
 * NOT      Not,                  [arg1] = ![arg2]
 * OR       Or,                   [arg1] =  [arg2] || [arg3]
 * AND
 * XOR
 * GTL      Greater Than Literal, [arg1] = ([arg2] >  arg3 )
 * GTF      Greater Than File,    [arg1] = ([arg2] > [arg3])
 * EQL
 * EQF
 * LTL
 * LTF
 * GTEL     Greater Than Or Equal Literal, [arg1] = ([arg2] >=  arg3 )
 * GTEF     Greater Than Or Equal File,    [arg1] = ([arg2] >= [arg3])
 * LTEL
 * LTEF
 */

template <typename T = uint8_t>
class mcu {
public:
    using reg_type = T;
    using func_type = void (mcu::*)(reg_type, reg_type, reg_type);
    
    const size_t rom_size;
    const size_t ram_size;
    const reg_type inssz = 4;
    
    reg_type * rom;
    reg_type * ram;
    
    double * pin;
    
    double v_max = 1e3;
    double v_min = -0.3;
    double v_on;
    
    enum sfr : reg_type {
        PC    = 0x00,
        SPTR  = 0x01,
        R0    = 0x02,
        R1    = 0x03,
        R2    = 0x04,
        R3    = 0x05,
        R4    = 0x06,
        R5    = 0x07,
        R6    = 0x08,
        R7    = 0x09,
        DDRA  = 0x0A,
        PORTA = 0x0B,
        PINA  = 0x0C,
        DDRB  = 0x0D,
        PORTB = 0x0E,
        PINB  = 0x0F
    };
    
    std::map<std::string, reg_type> sfr_map = {
        {"PC",    PC},
        {"SPTR",  SPTR},
        {"R0",    R0},
        {"R1",    R1},
        {"R2",    R2},
        {"R3",    R3},
        {"R4",    R4},
        {"R5",    R5},
        {"R6",    R6},
        {"R7",    R7},
        {"DDRA",  DDRA},
        {"PORTA", PORTA},
        {"PINA",  PINA},
        {"DDRB",  DDRB},
        {"PORTB", PORTB},
        {"PINB",  PINB},
    };
    
    enum pinout : reg_type {
        GND  =  0,
        VCC  =  1,
        NRST =  2,
        PA0  =  3,
        PA1  =  4,
        PA2  =  5,
        PA3  =  6,
        PA4  =  7,
        PA5  =  8,
        PA6  =  9,
        PA7  = 10,
        PB0  = 11,
        PB1  = 12,
        PB2  = 13,
        PB3  = 14,
        PB4  = 15,
        PB5  = 16,
        PB6  = 17,
        PB7  = 18
    };
    
    std::map<std::string, reg_type> pinout_map = {
        {"NRST", NRST},
        {"VCC",  VCC},
        {"GND",  GND},
        {"PA0",  PA0},
        {"PA1",  PA1},
        {"PA2",  PA2},
        {"PA3",  PA3},
        {"PA4",  PA4},
        {"PA5",  PA5},
        {"PA6",  PA6},
        {"PA7",  PA7},
        {"PB0",  PB0},
        {"PB1",  PB1},
        {"PB2",  PB2},
        {"PB3",  PB3},
        {"PB4",  PB4},
        {"PB5",  PB5},
        {"PB6",  PB6},
        {"PB7",  PB7},
    };
    
    std::map<std::string, reg_type> labels;
    std::map<std::string, reg_type> instructions;
    std::map<reg_type, func_type> functions;
    
    mcu(
        const size_t rom_size_inp, 
        const size_t ram_size_inp,
        const double v_on_inp = 4.5
    ):
        rom_size(rom_size_inp),
        ram_size(ram_size_inp)
    {
        rom = new reg_type[rom_size_inp];
        ram = new reg_type[ram_size_inp];
        pin = new double[2 * 8 * sizeof(reg_type) + 3];
        
        v_on = v_on_inp;
        
        pin[VCC]  = 5.0;
        pin[GND]  = 0.0;
        pin[NRST] = 5.0;
        
        load();
        reset();
    }
    
    ~mcu(){
        delete[] rom;
        delete[] ram;
        delete[] pin;
    }
    
    void add_func(const std::string & key, const reg_type id, func_type func){
        instructions[key] = id;
        functions[id] = func;
    }
    
    void load(){
        // regex find: {"(.+)"(,.+0x..)},?
        // regex replace: add_func("\1"\2, &\L\1);
        add_func("NOP",    0x00, &mcu<reg_type>::nop);
        add_func("MOVL",   0x01, &mcu<reg_type>::movl);
        add_func("MOVF",   0x02, &mcu<reg_type>::movf);
        add_func("CMOVL",  0x03, &mcu<reg_type>::cmovl);
        add_func("CMOVF",  0x04, &mcu<reg_type>::cmovf);
        add_func("CNMOVL", 0x05, &mcu<reg_type>::cnmovl);
        add_func("CNMOVF", 0x06, &mcu<reg_type>::cnmovf);
        add_func("RET",    0x07, &mcu<reg_type>::ret);
        add_func("ADDL",   0x10, &mcu<reg_type>::addl);
        add_func("ADDF",   0x11, &mcu<reg_type>::addf);
        add_func("SUBL",   0x12, &mcu<reg_type>::subl);
        add_func("SUBF",   0x13, &mcu<reg_type>::subf);
        add_func("MULL",   0x14, &mcu<reg_type>::mull);
        add_func("MULF",   0x15, &mcu<reg_type>::mulf);
        add_func("DIVL",   0x16, &mcu<reg_type>::divl);
        add_func("DIVF",   0x17, &mcu<reg_type>::divf);
        add_func("MODL",   0x18, &mcu<reg_type>::modl);
        add_func("MODF",   0x19, &mcu<reg_type>::modf);
        add_func("INC",    0x1A, &mcu<reg_type>::inc);
        add_func("DEC",    0x1B, &mcu<reg_type>::dec);
        add_func("AJ",     0x20, &mcu<reg_type>::aj);
        add_func("AJGTL",  0x21, &mcu<reg_type>::ajgtl);
        add_func("AJGTF",  0x22, &mcu<reg_type>::ajgtf);
        add_func("AJEQL",  0x23, &mcu<reg_type>::ajeql);
        add_func("AJEQF",  0x24, &mcu<reg_type>::ajeqf);
        add_func("AJLTL",  0x25, &mcu<reg_type>::ajltl);
        add_func("AJLTF",  0x26, &mcu<reg_type>::ajltf);
        add_func("AJGTEL", 0x27, &mcu<reg_type>::ajgtel);
        add_func("AJGTEF", 0x28, &mcu<reg_type>::ajgtef);
        add_func("AJLTEL", 0x29, &mcu<reg_type>::ajltel);
        add_func("AJLTEF", 0x2A, &mcu<reg_type>::ajltef);
        add_func("ACALL",  0x2B, &mcu<reg_type>::acall);
        add_func("RJ",     0x30, &mcu<reg_type>::rj);
        add_func("RJGTL",  0x31, &mcu<reg_type>::rjgtl);
        add_func("RJGTF",  0x32, &mcu<reg_type>::rjgtf);
        add_func("RJEQL",  0x33, &mcu<reg_type>::rjeql);
        add_func("RJEQF",  0x34, &mcu<reg_type>::rjeqf);
        add_func("RJLTL",  0x35, &mcu<reg_type>::rjltl);
        add_func("RJLTF",  0x36, &mcu<reg_type>::rjltf);
        add_func("RJGTEL", 0x37, &mcu<reg_type>::rjgtel);
        add_func("RJGTEF", 0x38, &mcu<reg_type>::rjgtef);
        add_func("RJLTEL", 0x39, &mcu<reg_type>::rjltel);
        add_func("RJLTEF", 0x3A, &mcu<reg_type>::rjltef);
        add_func("RCALL",  0x3B, &mcu<reg_type>::rcall);
        add_func("RRL",    0x40, &mcu<reg_type>::rrl);
        add_func("RRF",    0x41, &mcu<reg_type>::rrf);
        add_func("RLL",    0x42, &mcu<reg_type>::rll);
        add_func("RLF",    0x43, &mcu<reg_type>::rlf);
        add_func("BORL",   0x44, &mcu<reg_type>::borl);
        add_func("BORF",   0x45, &mcu<reg_type>::borf);
        add_func("BANDL",  0x46, &mcu<reg_type>::bandl);
        add_func("BANDF",  0x47, &mcu<reg_type>::bandf);
        add_func("BXORL",  0x48, &mcu<reg_type>::bxorl);
        add_func("BXORF",  0x49, &mcu<reg_type>::bxorf);
        add_func("BNOT",   0x4A, &mcu<reg_type>::bnot);
        add_func("NOT",    0x50, &mcu<reg_type>::f_not);
        add_func("OR",     0x52, &mcu<reg_type>::f_or);
        add_func("AND",    0x53, &mcu<reg_type>::f_and);
        add_func("XOR",    0x55, &mcu<reg_type>::f_xor);
        add_func("GTL",    0x56, &mcu<reg_type>::gtl);
        add_func("GTF",    0x57, &mcu<reg_type>::gtf);
        add_func("EQL",    0x58, &mcu<reg_type>::eql);
        add_func("EQF",    0x59, &mcu<reg_type>::eqf);
        add_func("LTL",    0x5A, &mcu<reg_type>::ltl);
        add_func("LTF",    0x5B, &mcu<reg_type>::ltf);
        add_func("GTEL",   0x5C, &mcu<reg_type>::gtel);
        add_func("GTEF",   0x5D, &mcu<reg_type>::gtef);
        add_func("LTEL",   0x5E, &mcu<reg_type>::ltel);
        add_func("LTEF",   0x5F, &mcu<reg_type>::ltef);
    }
    
    std::string which_instruction(reg_type value){
        auto it = find_if(instructions.begin(), instructions.end(), [value](auto & p) {return p.second == value;});
        
        if(it != instructions.end())
            return it->first;
        else
            return "";
    }
    
    bool match_number(const std::string & str, auto & num){
        static const std::regex number_regex("([\\+\\-])?(?:0([xb]))?((?:[\\da-f\\.]+)(?:e[\\+\\-]?\\d+)?)", std::regex_constants::icase | std::regex_constants::ECMAScript);
        std::smatch match;
        bool found;
        
        if(std::regex_match(str, match, number_regex)){
            std::string radix  = match[2].str();
            std::string number = match[1].str() + match[3].str();
            
            if(radix == "b" || radix == "B"){
                num = reg_type(std::stoull(number, 0, 0b10));
            }
            else if(radix == "x" || radix == "X"){
                num = reg_type(std::stoull(number, 0, 0x10));
            }
            else{
                num = reg_type(std::stoull(number, 0, 10));
            }
            
            found = true;
        }
        else{
            found = false;
        }
        
        return found;
    }
    
    reg_type parse(const std::string & str){
        reg_type ret;
        reg_type * reg_addr;
        
        static const std::regex label_regex(R"---((\S+):)---", std::regex_constants::icase | std::regex_constants::ECMAScript);
        std::smatch match;
        
        if(str != ""){
            if( (reg_addr = mfind(sfr_map, str)) ){
                ret = *reg_addr;
            }
            else if( (reg_addr = mfind(instructions, str)) ){
                ret = *reg_addr;
            }
            else if( (reg_addr = mfind(pinout_map, str)) ){
                ret = *reg_addr;
            }
            else if( (reg_addr = mfind(labels, str)) ){
                ret = *reg_addr;
            }
            else if(std::regex_match(str, match, label_regex)){
                
            }
            else if(str[0] == '\''){
                std::istringstream(str.substr(1, str.find('\'', 2))) >> ret;
            }
            else if(match_number(str, ret));
            else{
                fprintf(stderr, "[MCU] Failed to parse \"%s\"\n", str.c_str());
            }
        }
        else{
            ret = reg_type();
        }
        
        return ret;
    }
    
    void push_instruction(std::string & line, const bool restart = 0){
        std::string first;
        
        if(restart){
            end_ = 0;
        }
        if(end_ + 4 <= rom_size){
            first = next(line);
            if(first != ""){
                rom[end_++] = parse(first);
                rom[end_++] = parse(next(line));
                rom[end_++] = parse(next(line));
                rom[end_++] = parse(next(line));
            }
        }
    }
    
    void new_label(std::smatch & match, reg_type addr){
        labels[match[1].str()] = addr;
    }
    
    void read_stream(std::istream & stream){
        std::string line;
        std::string first;
        std::smatch match;
        
        static const std::regex label_regex(R"---((\S+):)---", std::regex_constants::icase | std::regex_constants::ECMAScript);
        static const std::regex ORG_regex(R"---(ORG\s*)---", std::regex_constants::icase | std::regex_constants::ECMAScript);
        
        // first pass: setup labels
        reg_type at = 0;
        while(std::getline(stream, line)){
            first = next(line);
            if(std::regex_match(first, match, label_regex)){
                new_label(match, at);
            }
            else if(std::regex_match(first, match, ORG_regex)){
                if(!match_number(next(line), at)){
                    fprintf(stderr, "[MCU] ORG expects a number\n");
                }
            }
            else if(first != ""){
                at += 4;
            }
        }
        // second pass: load instructions to ROM
        stream.clear();
        stream.seekg(0);
        at = 0;
        while(std::getline(stream, line)){
            first = next(line);
            if(std::regex_match(first, label_regex)){
                
            }
            else if(std::regex_match(first, match, ORG_regex)){
                match_number(next(line), at);
            }
            else if(first != ""){
                rom[at++] = parse(first);
                rom[at++] = parse(next(line));
                rom[at++] = parse(next(line));
                rom[at++] = parse(next(line));
            }
        }
    }
    
    std::string dump_rom(){
        size_t pos = 0;
        uint64_t value;
        char line[64];
        
        std::string output = "";
        
        while(pos < rom_size){
            value = rom[pos++];
            sprintf(line, "%2" PRIX64 " ", value);
            
            output += line;
            
            if(pos % 4 == 0)
                output += "[" + which_instruction(rom[pos - 4]) + "]\n";
        }
        return output;
    }
    
    void read_file(const std::string & filename){
        std::ifstream strm(filename);
        if(strm){
            read_stream(strm);
        }
        else{
            fprintf(stderr, "[MCU] Failed to open file \"%s\"\n", filename.c_str());
        }
    }
    
    void read_string(const std::string & src){
        std::istringstream strm(src);
        read_stream(strm);
    }
    
    void reset(){
        ram[PC]    = 0x00;
        ram[SPTR]  = reg_type(ram_size - 1);
        ram[DDRA]  = 0x00;
        ram[DDRB]  = 0x00;
        ram[PORTA] = 0x00;
        ram[PORTB] = 0x00;
    }
    
    void bitset(reg_type & reg, uint8_t bit){
        reg |= reg_type(  reg_type(0x01) << bit);
    }
    
    void bitclr(reg_type & reg, uint8_t bit){
        reg &= reg_type(~(reg_type(0x01) << bit));
    }
    
    void bitwr(reg_type & reg, uint8_t bit, uint8_t val){
        if(val)
            bitset(reg, bit);
        else
            bitclr(reg, bit);
    }
    
    void update_volatile(){
        uint8_t iterator = 0;
        while(iterator < 8){
            bitwr(ram[PINA], iterator, pin[PA0 + iterator] > pin[GND] + v_on/2);
            iterator++;
        }
        
        iterator = 0;
        while(iterator < 8){
            bitwr(ram[PINB], iterator, pin[PB0 + iterator] > pin[GND] + v_on/2);
            iterator++;
        }
    }
    
    void read_node_array(std::vector<node::node_t *> & nodes){
        size_t at = 0;
        for(const auto & node : nodes){
            pin[at] = node->voltage();
            at++;
        }
    }
    
    void write_driver_array(std::vector<mcu_gpio_driver_t> & gpio_drivers_) {
        reg_type bit;
        
        size_t at = 0;
        while(at < 8){
            bit = (1 << at);
            if(ram[DDRA] & bit){
                if(ram[PORTA] & bit){
                    gpio_drivers_[at].status(mcu_gpio_driver_t::HIGH);
                }
                else{
                    gpio_drivers_[at].status(mcu_gpio_driver_t::LOW);
                }
            }
            else{
                gpio_drivers_[at].status(mcu_gpio_driver_t::HI_Z);
            }
            at++;
        }
        
        at = 0;
        while(at < 8){
            bit = (1 << at);
            if(ram[DDRB] & bit){
                if(ram[PORTB] & bit){
                    gpio_drivers_[at + 8].status(mcu_gpio_driver_t::HIGH);
                }
                else{
                    gpio_drivers_[at + 8].status(mcu_gpio_driver_t::LOW);
                }
            }
            else{
                gpio_drivers_[at + 8].status(mcu_gpio_driver_t::HI_Z);
            }
            at++;
        }
    }
    
    void clock(){
        reg_type opcode = rom[ram[PC] + 0];
        reg_type arg0   = rom[ram[PC] + 1];
        reg_type arg1   = rom[ram[PC] + 2];
        reg_type arg2   = rom[ram[PC] + 3];
        
        if(pin[NRST] + pin[GND] < v_on){
            printf("[MCU] NRST Triggered\n");
            reset();
        }
        else if(pin[VCC] - pin[GND] >= v_on){
            update_volatile();
            
            func_type func;
            if(mfind(functions, opcode, func)){
                if(debug_){
                    printf("OP: %-6s %02lX %02lX %02lX\n", which_instruction(opcode).c_str(), uint64_t(arg0), uint64_t(arg1), uint64_t(arg2));
                }
                (this->*func)(arg0, arg1, arg2);
            }
            else
                fprintf(stderr, "[MCU] ERROR: Opcode %" PRIu64 " is invalid!\n", static_cast<uint64_t>(opcode));
            
            ram[PC] += inssz;
        }
        
    }
    
    void debug(const bool value){
        debug_ = value;
    }
    
private:
    reg_type end_ = 0;
    bool debug_ = false;
    
    void nop(reg_type, reg_type, reg_type){ }
    
    void movl(reg_type addr, reg_type val, reg_type){
        ram[addr] = val;
    }
    
    void movf(reg_type to, reg_type from, reg_type){
        ram[to] = ram[from];
    }
    
    void cmovl(reg_type addr, reg_type val, reg_type cond){
        if(ram[cond])
            ram[addr] = val;
    }
    
    void cmovf(reg_type to, reg_type from, reg_type cond){
        if(ram[cond])
            ram[to] = ram[from];
    }
    
    void cnmovl(reg_type addr, reg_type val, reg_type cond){
        if(!ram[cond])
            ram[addr] = val;
    }
    
    void cnmovf(reg_type to, reg_type from, reg_type cond){
        if(!ram[cond])
            ram[to] = ram[from];
    }
    
    void ret(reg_type, reg_type, reg_type){
        ram[SPTR]++;
        ram[PC] = ram[ram[SPTR]];
    }
    
    void addl(reg_type dest, reg_type a, reg_type b){
        ram[dest] = ram[a] + b;
    }
    
    void addf(reg_type dest, reg_type a, reg_type b){
        ram[dest] = ram[a] + ram[b];
    }
    
    void subl(reg_type dest, reg_type a, reg_type b){
        ram[dest] = ram[a] - b;
    }
    
    void subf(reg_type dest, reg_type a, reg_type b){
        ram[dest] = ram[a] - ram[b];
    }
    
    void mull(reg_type dest, reg_type a, reg_type b){
        ram[dest] = ram[a] * b;
    }
    
    void mulf(reg_type dest, reg_type a, reg_type b){
        ram[dest] = ram[a] * ram[b];
    }
    
    void divl(reg_type dest, reg_type a, reg_type b){
        ram[dest] = ram[a] / b;
    }
    
    void divf(reg_type dest, reg_type a, reg_type b){
        ram[dest] = ram[a] / ram[b];
    }
    
    void modl(reg_type dest, reg_type a, reg_type b){
        ram[dest] = ram[a] % b;
    }
    
    void modf(reg_type dest, reg_type a, reg_type b){
        ram[dest] = ram[a] % ram[b];
    }
    
    void inc(reg_type reg, reg_type, reg_type){
        ram[reg] = ram[reg] + 1;
    }
    
    void dec(reg_type reg, reg_type, reg_type){
        ram[reg] = ram[reg] - 1;
    }
    
    void aj(reg_type dest, reg_type, reg_type){
        ram[PC] = reg_type(dest - inssz);
    }
    
    void ajgtl(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] > rhs)
            ram[PC] = reg_type(dest - inssz);
    }
    
    void ajgtf(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] > ram[rhs])
            ram[PC] = reg_type(dest - inssz);
    }
    
    void ajeql(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] == rhs)
            ram[PC] = reg_type(dest - inssz);
    }
    
    void ajeqf(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] == ram[rhs])
            ram[PC] = reg_type(dest - inssz);
    }
    
    void ajltl(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] < rhs)
            ram[PC] = reg_type(dest - inssz);
    }
    
    void ajltf(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] < ram[rhs])
            ram[PC] = reg_type(dest - inssz);
    }
    
    void ajgtel(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] >= rhs)
            ram[PC] = reg_type(dest - inssz);
    }
    
    void ajgtef(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] >= ram[rhs])
            ram[PC] = reg_type(dest - inssz);
    }
    
    void ajltel(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] <= rhs)
            ram[PC] = reg_type(dest - inssz);
    }
    
    void ajltef(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] <= ram[rhs])
            ram[PC] = reg_type(dest - inssz);
    }
    
    void acall(reg_type dest, reg_type, reg_type){
        ram[ram[SPTR]] = ram[PC];
        ram[SPTR]--;
        ram[PC] = reg_type(dest - inssz);
    }
    
    void rj(reg_type dest, reg_type, reg_type){
        ram[PC] += reg_type(dest - inssz);
    }
    
    void rjgtl(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] > rhs)
            ram[PC] += reg_type(dest - inssz);
    }
    
    void rjgtf(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] > ram[rhs])
            ram[PC] += reg_type(dest - inssz);
    }
    
    void rjeql(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] == rhs)
            ram[PC] += reg_type(dest - inssz);
    }
    
    void rjeqf(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] == ram[rhs])
            ram[PC] += reg_type(dest - inssz);
    }
    
    void rjltl(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] < rhs)
            ram[PC] += reg_type(dest - inssz);
    }
    
    void rjltf(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] < ram[rhs])
            ram[PC] += reg_type(dest - inssz);
    }
    
    void rjgtel(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] >= rhs)
            ram[PC] += reg_type(dest - inssz);
    }
    
    void rjgtef(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] >= ram[rhs])
            ram[PC] += reg_type(dest - inssz);
    }
    
    void rjltel(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] <= rhs)
            ram[PC] += reg_type(dest - inssz);
    }
    
    void rjltef(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram[lhs] <= ram[rhs])
            ram[PC] += reg_type(dest - inssz);
    }
    
    void rcall(reg_type dest, reg_type, reg_type){
        ram[ram[SPTR]] = ram[PC];
        ram[SPTR]--;
        ram[PC] += reg_type(dest - inssz);
    }
    
    void rrl(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = reg_type(ram[lhs] >> rhs);
    }
    
    void rrf(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = reg_type(ram[lhs] >> ram[rhs]);
    }
    
    void rll(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = reg_type(ram[lhs] << rhs);
    }

    void rlf(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = reg_type(ram[lhs] << ram[rhs]);
    }
    
    void borl(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] | rhs;
    }

    void borf(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] | ram[rhs];
    }

    void bandl(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] & rhs;
    }
    
    void bandf(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] & ram[rhs];
    }
    
    void bxorl(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] ^ rhs;
    }
    
    void bxorf(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] ^ ram[rhs];
    }

    void bnot(reg_type dest, reg_type src, reg_type){
        ram[dest] = ~ram[src];
    }

    void f_not(reg_type dest, reg_type src, reg_type){
        ram[dest] = !ram[src];
    }
    
    void f_or(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] || ram[rhs];
    }
    
    void f_and(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] && ram[rhs];
    }
    
    void f_xor(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] != ram[rhs];
    }
    
    void gtl(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] > rhs;
    }
    
    void gtf(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] > ram[rhs];
    }
    
    void eql(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] == rhs;
    }
    
    void eqf(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] == ram[rhs];
    }
    
    void ltl(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] < rhs;
    }
    
    void ltf(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] < ram[rhs];
    }
    
    void gtel(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] >= rhs;
    }
    
    void gtef(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] >= ram[rhs];
    }
    
    void ltel(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] <= rhs;
    }
    
    void ltef(reg_type dest, reg_type lhs, reg_type rhs){
        ram[dest] = ram[lhs] <= ram[rhs];
    }
    
};


static constexpr const char * mcu_help_string = 
R"---(digital::mcu 0.6.1 2021-09-12

|------------|--------|---------|------|
|  argument  |  type  | default | unit |
|============|========|=========|======|
| ROM size   | size_t |     256 | B    |
| RAM size   | size_t |     256 | B    |
| ASM file   | string |         |      |
| debug rom  | bool   |   false |      |
| debug op   | bool   |   false |      |
| frequency  | double |      1M | Hz   |
| GPIO res   | double |      10 | Ohm  |
|------------|--------|---------|------|

=====[ printables ]=====

=====[ CLI arguments ]=====
ROM size:
  The size (in Bytes) of the Read Only Memory.
  The ROM stores instructions.

RAM size:
  The size (in Bytes) of the Read Only Memory.
  The RAM stores data (variables).

ASM file:
  The relative path/name to the file containing the assembly code.

debug rom:
  Accepts "true" or "false". If true, when the compilation is done,
  the ROM contents will be printed to the console.

debug op:
  Accepts "true" or "false". Prints the instruction and args.

frequency:
  How many instructions will be executed per simulation-second. In Hz.

GPIO res:
  The GPIO series resistance. Limits the current flowing from the pins.

=====[ description ]=====
Every instruction uses 4 bytes:
  opcode arg1 arg2 arg3
  
Disclaimer:
  [argX] represents the memory location pointed by argX
  
The instructions are:
  NOP      No operation
  MOVL     Move Literal,   move the litreral arg2  to [arg1]
  MOVF     Move File,      move             [arg2] to [arg1]
  CMOVL    Conditional Move Literal,       move the litreral arg2  to [arg1] if [arg3] != 0
  CMOVF    Conditional Move File,          move             [arg2] to [arg1] if [arg3] != 0
  CNMOVL   Conditional Not Move Literal,   move the litreral arg2  to [arg1] if [arg3] == 0
  CNMOVF   Conditional Not Move File,      move             [arg2] to [arg1] if [arg3] == 0
  RET      Return,         returns from a call, popping the stack
  ADDL     Add Literal,    add the literal  arg2  to [arg1] and insert the result in [arg3]
  ADDF     Add File,       add the literal [arg2] to [arg1] and insert the result in [arg3]
  SUBL
  SUBF
  MULL
  MULF
  DIVL
  DIVF
  MODL     Modulo Literal, [arg1] = [arg2] %  arg3
  MODF     Module File,    [arg1] = [arg2] % [arg3]        
  INC      Increment,      add 1 to [arg1]
  DEC
  AJ       Absolute Jump,  Jump inconditionally to the literal address arg1
  AJGTL    Absolute jump if greater than literal, jump to [arg1] if [arg2] > literal arg3
  AJGTF    Absolute jump if greater than file,    jump to [arg1] if [arg2] > [arg3]
  AJEQL    --               equal        literal
  AJEQF    --               equal        file
  AJLTL    --               less than    literal
  AJLTF    --               less than    file
  AJGTEL   --               greater than or equal literal
  AJGTEF   --               greater than or equal file
  AJLTEL   --               less than or equal    literal
  AJLTEF   --               less than or equal    file
  ACALL    Absolute Call,  stores the PC in the stack and jumps to the literal address arg1
  RJ       Relative Jump,  Jump inconditionally to PC + arg1
  RJGTL
  RJGTF
  RJEQL
  RJEQF
  RJLTL
  RJLTF
  RJGTEL
  RJGTEF
  RJLTEL
  RJLTEF
  RCALL
  RRL      Rotate Right Literal, Rotate [arg2] literal arg3 times and insert the result in [arg1]      [arg1] = [arg2] >>  arg3 
  RRF      Rotate Right File,    Rotate [arg2] [arg3] times and insert the result in [arg1]            [arg1] = [arg2] >> [arg3]
  RLL
  RLF
  BORL     Binary Or Literal,    [arg1] = [arg2] |  arg3
  BORF     Binary Or File,       [arg1] = [arg2] | [arg3]
  BANDL
  BANDF
  BXORL
  BXORF
  BNOT     Binary not,           [arg1] = ~[arg2]
  NOT      Not,                  [arg1] = ![arg2]
  OR       Or,                   [arg1] =  [arg2] || [arg3]
  AND
  XOR
  GTL      Greater Than Literal, [arg1] = ([arg2] >  arg3 )
  GTF      Greater Than File,    [arg1] = ([arg2] > [arg3])
  EQL
  EQF
  LTL
  LTF
  GTEL     Greater Than Or Equal Literal, [arg1] = ([arg2] >=  arg3 )
  GTEF     Greater Than Or Equal File,    [arg1] = ([arg2] >= [arg3])
  LTEL
  LTEF
)---";

template <typename T>
class sim_mcu_t : public comp::component_t {
public:
    using reg_type = T;
    using mcu_type = mcu<reg_type>;
    
    std::stringstream configure(node_map_type & nmap, printable_map_type &, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        size_t rom_size   = next_as<size_t>(args, 0x100);
        size_t ram_size   = next_as<size_t>(args, 0x100);
        source_file_name_ = next(args);
        bool debug_rom    = next_as<bool>(args, false);
        bool debug_op     = next_as<bool>(args, false);
        double frequency  = next_as<double>(args, 1e6);
        double v_on       = next_as<double>(args, 2.5);
        gpio_impedance_   = next_as<double>(args, 10);
        

        nodes_ = std::vector<node::node_t *>(pin_count_);
        size_t iter = pin_count_;
        while(iter--){
            const std::string node_name = this->label() + ":" + std::to_string(iter + 1);
            node::node_t * & node = nodes_[iter];
            
            info << utils::make_node_s(nmap, node_name, node, error);
            if(error) return info;
            
            std::cout << "creating " << this->label() + ":" + std::to_string(iter + 1) << " => " << &(nodes_[iter]->current_in_) << "\n";
            
            node->capacitance(period_ / (5 * gpio_impedance_)); // T99 = 5 * R * C
            node->is_hidden(true);
        }
        
        gpio_drivers_ = std::vector<mcu_gpio_driver_t>(gpio_count_);
        period_ = 1/frequency;
        
        mcu_ = std::make_unique<mcu_type>(rom_size, ram_size, v_on);
        mcu_->debug(debug_op);
        mcu_->read_file(source_file_name_);
        if(debug_rom){
            printf("%s\n", mcu_->dump_rom().c_str());
        }
        
        return info;
    }
    
    void clear_variables(){
        next_clock_time_ = period_;
        next_clock_diff_ = 0;
        
        for(auto & dri : gpio_drivers_){
            dri.clear_variables();
        }
    }
    
    void setup(sim::simulator_t * simulator, const double voltage_difference_max, const double time_step_max) override {
        size_t at = 0;
        while(at < 8){
            gpio_drivers_[at].label("MCU gpio driver PA" + std::to_string(at));
            gpio_drivers_[at].set_nodes(nodes_[mcu_type::VCC], nodes_[mcu_type::GND], nodes_[mcu_type::PA0 + at]);
            gpio_drivers_[at].resistance(gpio_impedance_);
            at++;
        }
        at = 0;
        while(at < 8){
            gpio_drivers_[at + 8].label("MCU gpio driver PB" + std::to_string(at));
            gpio_drivers_[at + 8].set_nodes(nodes_[mcu_type::VCC], nodes_[mcu_type::GND], nodes_[mcu_type::PB0 + at]);
            gpio_drivers_[at + 8].resistance(gpio_impedance_);
            at++;
        }
        
        for(auto & dri : gpio_drivers_){
            dri.setup(simulator, voltage_difference_max, time_step_max);
        }
    }
    
    void update(double time, double last_time_step) override {
        if(time >= next_clock_time_){
            next_clock_time_ += period_;
            mcu_->read_node_array(nodes_);
            mcu_->clock();
            mcu_->write_driver_array(gpio_drivers_);
        }
        next_clock_diff_ = next_clock_time_ - time;
        
        std::cout << "updated (" << last_time_step*1e9 << "ns)\n";
        for(auto & dri : gpio_drivers_){
            dri.update(time, last_time_step);
            std::cout << dri.current() << "\n";
        }
        
        for(auto * node : nodes_) std::cout << node->label() << ": " << node->current_in() << "\n";
    }
    
    double time_step_max(const double /*time*/) const override {
        return next_clock_diff_;
    }
    
    const char * help() const override {
        return mcu_help_string;
    }
    
private:
    // sim var
    double next_clock_time_ = 0;
    double next_clock_diff_ = 0;
    
    // params
    double period_;
    double gpio_impedance_;
    std::string source_file_name_ = "";
    
    // internal
    std::shared_ptr<mcu_type> mcu_;
    const size_t gpio_count_ = 2 * 8 * sizeof(reg_type);
    const size_t pin_count_ = gpio_count_ + 3;
    std::vector<node::node_t *> nodes_;
    std::vector<mcu_gpio_driver_t> gpio_drivers_;
};

// not included until
// gpio driver bug is fixed
// autoreg::reg<sim_mcu_t<uint8_t>> mcu8_reg(database::components(), "digital::mcu8");
