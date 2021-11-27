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
#include <thread>
#include <chrono>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../libs/mfind.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"
#include "../libs/bit.hpp"

namespace mcu_v2 {

template <typename T>
class gpio_t {
public:
    using reg_type = T;
    using bit_type = bit_t<reg_type>;
    
    static constexpr bool low = false;
    static constexpr bool high = true;
    static constexpr bool input = false;
    static constexpr bool output = true;
    
    gpio_t(
        node::node_t * node_positive,
        node::node_t * node_negative,
        node::node_t * node_pin,
        reg_type & register_write,
        reg_type & register_read,
        reg_type & register_direction,
        reg_type bit_number,
        const double voltage_low,
        const double voltage_high,
        const double resistance
    ) :
        voltage_low_(voltage_low),
        voltage_high_(voltage_high),
        resistance_(resistance)
    {
        node_positive_ = node_positive;
        node_negative_ = node_negative;
        node_pin_      = node_pin;
        
        bit_data_out_  = bit_type(&register_write, bit_number);
        bit_data_in_   = bit_type(&register_read, bit_number);
        bit_direction_ = bit_type(&register_direction, bit_number);
    }
    
    void update_node() {
        const double pin_voltage = voltage_difference(node_pin_, node_negative_);
        
        if(pin_voltage > voltage_high_) {
            bit_data_in_ = high;
        }
        else if(pin_voltage <= voltage_low_) {
            bit_data_in_ = low;
        }
        
        if(node_pin_ != nullptr && bit_direction_ == output) {
            if(bit_data_out_ == high) {
                const double current_pin_out = voltage_difference(node_positive_, node_pin_) / resistance_;
                //std::cout << "\n<<@mcu HIGH : " << voltage_difference(node_positive_, node_pin_) << "/" << resistance_ << " = " << current_pin_out << ">>\n";
                node_pin_->current_in(current_pin_out);
                node_positive_->current_out(current_pin_out);
            }
            else {
                const double current_pin_out = voltage_difference(node_negative_, node_pin_) / resistance_;
                //std::cout << "\n<<@mcu LOW : " << voltage_difference(node_negative_, node_pin_) << "/" << resistance_ << " = " << current_pin_out << ">>\n";
                node_pin_->current_in(current_pin_out);
                node_negative_->current_out(current_pin_out);
            }
        }
    }
    
private:
    bit_type bit_data_out_;
    bit_type bit_data_in_;
    bit_type bit_direction_;
    
    const double voltage_low_;
    const double voltage_high_;
    
    double resistance_;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
    node::node_t * node_pin_;
};

template <typename T>
class adc_t {
public:
    using reg_type = T;
    
    adc_t(
        node::node_t * node_ref,
        node::node_t * node_pin,
        reg_type * register_result,
        const double voltage_range
    ) {
        node_ref_ = node_ref;
        node_pin_ = node_pin;
        
        register_result_ = register_result;
        
        voltage_range_ = voltage_range;
    }
    
    void update() {
        static constexpr double value_max = std::numeric_limits<reg_type>::max();
        
        const double voltage = voltage_difference(node_pin_, node_ref_);
        const double voltage_scaled = value_max * voltage / voltage_range_;
        const double voltage_clamp = std::clamp(voltage_scaled, 0.0, value_max);
        
        *register_result_ = reg_type(voltage_clamp + 0.5);
    }
    
private:
    reg_type * register_result_;
    double voltage_range_;
    
    node::node_t * node_ref_;
    node::node_t * node_pin_;
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
 * MOVL:    Move Literal,          move the litreral arg2   to  [arg1]
 * MOVF:    Move File,             move             [arg2]  to  [arg1]
 * MOVPF:   Move [File] to  File,  move            [[arg2]] to  [arg1]
 * MOVFP:   Move  File  to [File], move             [arg2]  to [[arg1]]
 * MOVPP:   Move [File] to [File], move            [[arg2]] to [[arg1]]
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

template <typename T>
class microcontroller_t {
public:
    using reg_type = T;
    //using gpio_type = gpio_t<reg_type>;
    
    using rom_type = std::vector<uint8_t>;
    using ram_type = std::vector<uint8_t>;
    
    using func_type = void (microcontroller_t::*)(reg_type, reg_type, reg_type);
    
    size_t rom_size;
    size_t ram_size;
    
    static constexpr reg_type inssz = 4;
    
    microcontroller_t() = default;
    
    microcontroller_t(const size_t rom_size_bytes, const size_t ram_size_bytes, const bool debug) {
        rom_size = rom_size_bytes;
        ram_size = ram_size_bytes;
        
        rom_ = rom_type(rom_size_bytes);
        ram_ = ram_type(ram_size_bytes);
        
        is_debugging_ = debug;
        
        load_instructions();
    }
    
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
        PINB  = 0x0F,
        ADC0  = 0x10,
        ADC1  = 0x11,
        ADC2  = 0x12,
        ADC3  = 0x13,
        ADC4  = 0x14,
        ADC5  = 0x15,
        ADC6  = 0x16,
        ADC7  = 0x17
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
        {"ADC0",  ADC0},
        {"ADC1",  ADC1},
        {"ADC2",  ADC2},
        {"ADC3",  ADC3},
        {"ADC4",  ADC4},
        {"ADC5",  ADC5},
        {"ADC6",  ADC6},
        {"ADC7",  ADC7},
    };
    
    enum pinout : reg_type {
        GND  =  0,
        VCC  =  1,
        NRST =  2
    };
    
    std::map<std::string, reg_type> pinout_map = {
        {"NRST", NRST},
        {"VCC",  VCC},
        {"GND",  GND}
    };
    
    std::map<std::string, reg_type> labels;
    std::map<std::string, reg_type> instructions;
    std::map<reg_type, func_type> functions;
    
    void add_func(const std::string & key, const reg_type id, func_type func) {
        instructions[key] = id;
        functions[id] = func;
    }
    
    
    void load_instructions() {
        add_func("NOP",    0x00, &microcontroller_t<reg_type>::nop);
        add_func("MOVL",   0x01, &microcontroller_t<reg_type>::movl);
        add_func("MOVF",   0x02, &microcontroller_t<reg_type>::movf);
        add_func("MOVPF",  0x03, &microcontroller_t<reg_type>::movpf);
        add_func("MOVFP",  0x04, &microcontroller_t<reg_type>::movfp);
        add_func("MOVPP",  0x05, &microcontroller_t<reg_type>::movpp);
        add_func("CMOVL",  0x06, &microcontroller_t<reg_type>::cmovl);
        add_func("CMOVF",  0x07, &microcontroller_t<reg_type>::cmovf);
        add_func("CNMOVL", 0x08, &microcontroller_t<reg_type>::cnmovl);
        add_func("CNMOVF", 0x09, &microcontroller_t<reg_type>::cnmovf);
        add_func("RET",    0x0A, &microcontroller_t<reg_type>::ret);
        add_func("ADDL",   0x10, &microcontroller_t<reg_type>::addl);
        add_func("ADDF",   0x11, &microcontroller_t<reg_type>::addf);
        add_func("SUBL",   0x12, &microcontroller_t<reg_type>::subl);
        add_func("SUBF",   0x13, &microcontroller_t<reg_type>::subf);
        add_func("MULL",   0x14, &microcontroller_t<reg_type>::mull);
        add_func("MULF",   0x15, &microcontroller_t<reg_type>::mulf);
        add_func("DIVL",   0x16, &microcontroller_t<reg_type>::divl);
        add_func("DIVF",   0x17, &microcontroller_t<reg_type>::divf);
        add_func("MODL",   0x18, &microcontroller_t<reg_type>::modl);
        add_func("MODF",   0x19, &microcontroller_t<reg_type>::modf);
        add_func("INC",    0x1A, &microcontroller_t<reg_type>::inc);
        add_func("DEC",    0x1B, &microcontroller_t<reg_type>::dec);
        add_func("AJ",     0x20, &microcontroller_t<reg_type>::aj);
        add_func("AJGTL",  0x21, &microcontroller_t<reg_type>::ajgtl);
        add_func("AJGTF",  0x22, &microcontroller_t<reg_type>::ajgtf);
        add_func("AJEQL",  0x23, &microcontroller_t<reg_type>::ajeql);
        add_func("AJEQF",  0x24, &microcontroller_t<reg_type>::ajeqf);
        add_func("AJLTL",  0x25, &microcontroller_t<reg_type>::ajltl);
        add_func("AJLTF",  0x26, &microcontroller_t<reg_type>::ajltf);
        add_func("AJGTEL", 0x27, &microcontroller_t<reg_type>::ajgtel);
        add_func("AJGTEF", 0x28, &microcontroller_t<reg_type>::ajgtef);
        add_func("AJLTEL", 0x29, &microcontroller_t<reg_type>::ajltel);
        add_func("AJLTEF", 0x2A, &microcontroller_t<reg_type>::ajltef);
        add_func("ACALL",  0x2B, &microcontroller_t<reg_type>::acall);
        add_func("RJ",     0x30, &microcontroller_t<reg_type>::rj);
        add_func("RJGTL",  0x31, &microcontroller_t<reg_type>::rjgtl);
        add_func("RJGTF",  0x32, &microcontroller_t<reg_type>::rjgtf);
        add_func("RJEQL",  0x33, &microcontroller_t<reg_type>::rjeql);
        add_func("RJEQF",  0x34, &microcontroller_t<reg_type>::rjeqf);
        add_func("RJLTL",  0x35, &microcontroller_t<reg_type>::rjltl);
        add_func("RJLTF",  0x36, &microcontroller_t<reg_type>::rjltf);
        add_func("RJGTEL", 0x37, &microcontroller_t<reg_type>::rjgtel);
        add_func("RJGTEF", 0x38, &microcontroller_t<reg_type>::rjgtef);
        add_func("RJLTEL", 0x39, &microcontroller_t<reg_type>::rjltel);
        add_func("RJLTEF", 0x3A, &microcontroller_t<reg_type>::rjltef);
        add_func("RCALL",  0x3B, &microcontroller_t<reg_type>::rcall);
        add_func("RRL",    0x40, &microcontroller_t<reg_type>::rrl);
        add_func("RRF",    0x41, &microcontroller_t<reg_type>::rrf);
        add_func("RLL",    0x42, &microcontroller_t<reg_type>::rll);
        add_func("RLF",    0x43, &microcontroller_t<reg_type>::rlf);
        add_func("BORL",   0x44, &microcontroller_t<reg_type>::borl);
        add_func("BORF",   0x45, &microcontroller_t<reg_type>::borf);
        add_func("BANDL",  0x46, &microcontroller_t<reg_type>::bandl);
        add_func("BANDF",  0x47, &microcontroller_t<reg_type>::bandf);
        add_func("BXORL",  0x48, &microcontroller_t<reg_type>::bxorl);
        add_func("BXORF",  0x49, &microcontroller_t<reg_type>::bxorf);
        add_func("BNOT",   0x4A, &microcontroller_t<reg_type>::bnot);
        add_func("NOT",    0x50, &microcontroller_t<reg_type>::f_not);
        add_func("OR",     0x52, &microcontroller_t<reg_type>::f_or);
        add_func("AND",    0x53, &microcontroller_t<reg_type>::f_and);
        add_func("XOR",    0x55, &microcontroller_t<reg_type>::f_xor);
        add_func("GTL",    0x56, &microcontroller_t<reg_type>::gtl);
        add_func("GTF",    0x57, &microcontroller_t<reg_type>::gtf);
        add_func("EQL",    0x58, &microcontroller_t<reg_type>::eql);
        add_func("EQF",    0x59, &microcontroller_t<reg_type>::eqf);
        add_func("LTL",    0x5A, &microcontroller_t<reg_type>::ltl);
        add_func("LTF",    0x5B, &microcontroller_t<reg_type>::ltf);
        add_func("GTEL",   0x5C, &microcontroller_t<reg_type>::gtel);
        add_func("GTEF",   0x5D, &microcontroller_t<reg_type>::gtef);
        add_func("LTEL",   0x5E, &microcontroller_t<reg_type>::ltel);
        add_func("LTEF",   0x5F, &microcontroller_t<reg_type>::ltef);
    }
    
    std::string which_instruction(reg_type value) {
        auto it = find_if(instructions.begin(), instructions.end(), [value](auto & p) {return p.second == value;});
        
        if(it != instructions.end())
            return it->first;
        else
            return "";
    }
    
    bool match_number(const std::string & str, auto & num) {
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
    
    reg_type parse(const std::string & str) {
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
    
    void push_instruction(std::string & line, const bool restart = 0) {
        std::string first;
        
        if(restart){
            last_instruction_ = 0;
        }
        if(last_instruction_ + 4 <= rom_size){
            first = next(line);
            if(first != ""){
                rom_[last_instruction_++] = parse(first);
                rom_[last_instruction_++] = parse(next(line));
                rom_[last_instruction_++] = parse(next(line));
                rom_[last_instruction_++] = parse(next(line));
            }
        }
    }
    
    void new_label(std::smatch & match, reg_type addr) {
        labels[match[1].str()] = addr;
    }
    
    void read_stream(std::istream & stream) {
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
                rom_[at++] = parse(first);
                rom_[at++] = parse(next(line));
                rom_[at++] = parse(next(line));
                rom_[at++] = parse(next(line));
            }
        }
    }
    
    std::string dump_rom() {
        size_t pos = 0;
        uint64_t value;
        char line[64];
        
        std::string output = "";
        
        while(pos < rom_size){
            value = rom_[pos++];
            sprintf(line, "%2" PRIX64 " ", value);
            
            output += line;
            
            if(pos % 4 == 0)
                output += "[" + which_instruction(rom_[pos - 4]) + "]\n";
        }
        return output;
    }
    
    void read_file(const std::string & filename) {
        std::ifstream strm(filename);
        if(strm){
            read_stream(strm);
        }
        else{
            fprintf(stderr, "[MCU] Failed to open file \"%s\"\n", filename.c_str());
        }
    }
    
    void read_string(const std::string & src) {
        std::istringstream strm(src);
        read_stream(strm);
    }
    
    void reset() {
        ram_[PC]    = 0x00;
        ram_[SPTR]  = reg_type(ram_size - 1);
        ram_[DDRA]  = 0x00;
        ram_[DDRB]  = 0x00;
        ram_[PORTA] = 0x00;
        ram_[PORTB] = 0x00;
    }
    
    void bitset(reg_type & reg, uint8_t bit) {
        reg |= reg_type(  reg_type(0x01) << bit);
    }
    
    void bitclr(reg_type & reg, uint8_t bit) {
        reg &= reg_type(~(reg_type(0x01) << bit));
    }
    
    void bitwr(reg_type & reg, uint8_t bit, uint8_t val) {
        if(val)
            bitset(reg, bit);
        else
            bitclr(reg, bit);
    }
    
    void clock() {
        reg_type opcode = rom_[ram_[PC] + 0];
        reg_type arg0   = rom_[ram_[PC] + 1];
        reg_type arg1   = rom_[ram_[PC] + 2];
        reg_type arg2   = rom_[ram_[PC] + 3];
        
        func_type func;
        if(mfind(functions, opcode, func)){
            if(is_debugging_){
                printf("PC: %4ld SC: %4ld SP: %4ld | OP: %-6s %02lX %02lX %02lX\n", 
                    uint64_t(ram_[PC]), 
                    uint64_t(ram_[SPTR]), 
                    uint64_t(ram_[ram_[SPTR] + 1]), 
                    which_instruction(opcode).c_str(), 
                    uint64_t(arg0), 
                    uint64_t(arg1), 
                    uint64_t(arg2)
                );
            }
            (this->*func)(arg0, arg1, arg2);
        }
        else
            fprintf(stderr, "[MCU] ERROR: Opcode %" PRIu64 " is invalid!\n", static_cast<uint64_t>(opcode));
        
        ram_[PC] += inssz;
    }
    
    rom_type & rom() {
        return rom_;
    }
    
    reg_type & rom(size_t pos) {
        return rom_[pos];
    }
    
    ram_type & ram() {
        return ram_;
    }
    
    reg_type & ram(size_t pos) {
        return ram_[pos];
    }
    
private:
    rom_type rom_;
    ram_type ram_;
    
    reg_type data_bus;
    reg_type instruction_bus;
    
    size_t last_instruction_ = 0;
    bool is_debugging_;
    
    void nop(reg_type, reg_type, reg_type){ }
    
    void movl(reg_type addr, reg_type val, reg_type){
        ram_[addr] = val;
    }
    
    void movf(reg_type to, reg_type from, reg_type){
        ram_[to] = ram_[from];
    }
    
    void movpf(reg_type to, reg_type from, reg_type){
        ram_[to] = ram_[ram_[from]];
    }
    
    void movfp(reg_type to, reg_type from, reg_type){
        ram_[ram_[to]] = ram_[from];
    }
    
    void movpp(reg_type to, reg_type from, reg_type){
        ram_[ram_[to]] = ram_[ram_[from]];
    }
    
    void cmovl(reg_type addr, reg_type val, reg_type cond){
        if(ram_[cond])
            ram_[addr] = val;
    }
    
    void cmovf(reg_type to, reg_type from, reg_type cond){
        if(ram_[cond])
            ram_[to] = ram_[from];
    }
    
    void cnmovl(reg_type addr, reg_type val, reg_type cond){
        if(!ram_[cond])
            ram_[addr] = val;
    }
    
    void cnmovf(reg_type to, reg_type from, reg_type cond){
        if(!ram_[cond])
            ram_[to] = ram_[from];
    }
    
    void ret(reg_type, reg_type, reg_type){
        ram_[SPTR]++;
        ram_[PC] = ram_[ram_[SPTR]];
    }
    
    void addl(reg_type dest, reg_type a, reg_type b){
        ram_[dest] = ram_[a] + b;
    }
    
    void addf(reg_type dest, reg_type a, reg_type b){
        ram_[dest] = ram_[a] + ram_[b];
    }
    
    void subl(reg_type dest, reg_type a, reg_type b){
        ram_[dest] = ram_[a] - b;
    }
    
    void subf(reg_type dest, reg_type a, reg_type b){
        ram_[dest] = ram_[a] - ram_[b];
    }
    
    void mull(reg_type dest, reg_type a, reg_type b){
        ram_[dest] = ram_[a] * b;
    }
    
    void mulf(reg_type dest, reg_type a, reg_type b){
        ram_[dest] = ram_[a] * ram_[b];
    }
    
    void divl(reg_type dest, reg_type a, reg_type b){
        ram_[dest] = ram_[a] / b;
    }
    
    void divf(reg_type dest, reg_type a, reg_type b){
        ram_[dest] = ram_[a] / ram_[b];
    }
    
    void modl(reg_type dest, reg_type a, reg_type b){
        ram_[dest] = ram_[a] % b;
    }
    
    void modf(reg_type dest, reg_type a, reg_type b){
        ram_[dest] = ram_[a] % ram_[b];
    }
    
    void inc(reg_type reg, reg_type, reg_type){
        ram_[reg] = ram_[reg] + 1;
    }
    
    void dec(reg_type reg, reg_type, reg_type){
        ram_[reg] = ram_[reg] - 1;
    }
    
    void aj(reg_type dest, reg_type, reg_type){
        ram_[PC] = reg_type(dest - inssz);
    }
    
    void ajgtl(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] > rhs)
            ram_[PC] = reg_type(dest - inssz);
    }
    
    void ajgtf(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] > ram_[rhs])
            ram_[PC] = reg_type(dest - inssz);
    }
    
    void ajeql(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] == rhs)
            ram_[PC] = reg_type(dest - inssz);
    }
    
    void ajeqf(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] == ram_[rhs])
            ram_[PC] = reg_type(dest - inssz);
    }
    
    void ajltl(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] < rhs)
            ram_[PC] = reg_type(dest - inssz);
    }
    
    void ajltf(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] < ram_[rhs])
            ram_[PC] = reg_type(dest - inssz);
    }
    
    void ajgtel(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] >= rhs)
            ram_[PC] = reg_type(dest - inssz);
    }
    
    void ajgtef(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] >= ram_[rhs])
            ram_[PC] = reg_type(dest - inssz);
    }
    
    void ajltel(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] <= rhs)
            ram_[PC] = reg_type(dest - inssz);
    }
    
    void ajltef(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] <= ram_[rhs])
            ram_[PC] = reg_type(dest - inssz);
    }
    
    void acall(reg_type dest, reg_type, reg_type){
        ram_[ram_[SPTR]] = ram_[PC];
        ram_[SPTR]--;
        ram_[PC] = reg_type(dest - inssz);
    }
    
    void rj(reg_type dest, reg_type, reg_type){
        ram_[PC] += reg_type(dest - inssz);
    }
    
    void rjgtl(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] > rhs)
            ram_[PC] += reg_type(dest - inssz);
    }
    
    void rjgtf(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] > ram_[rhs])
            ram_[PC] += reg_type(dest - inssz);
    }
    
    void rjeql(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] == rhs)
            ram_[PC] += reg_type(dest - inssz);
    }
    
    void rjeqf(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] == ram_[rhs])
            ram_[PC] += reg_type(dest - inssz);
    }
    
    void rjltl(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] < rhs)
            ram_[PC] += reg_type(dest - inssz);
    }
    
    void rjltf(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] < ram_[rhs])
            ram_[PC] += reg_type(dest - inssz);
    }
    
    void rjgtel(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] >= rhs)
            ram_[PC] += reg_type(dest - inssz);
    }
    
    void rjgtef(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] >= ram_[rhs])
            ram_[PC] += reg_type(dest - inssz);
    }
    
    void rjltel(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] <= rhs)
            ram_[PC] += reg_type(dest - inssz);
    }
    
    void rjltef(reg_type dest, reg_type lhs, reg_type rhs){
        if(ram_[lhs] <= ram_[rhs])
            ram_[PC] += reg_type(dest - inssz);
    }
    
    void rcall(reg_type dest, reg_type, reg_type){
        ram_[ram_[SPTR]] = ram_[PC];
        ram_[SPTR]--;
        ram_[PC] += reg_type(dest - inssz);
    }
    
    void rrl(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = reg_type(ram_[lhs] >> rhs);
    }
    
    void rrf(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = reg_type(ram_[lhs] >> ram_[rhs]);
    }
    
    void rll(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = reg_type(ram_[lhs] << rhs);
    }

    void rlf(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = reg_type(ram_[lhs] << ram_[rhs]);
    }
    
    void borl(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] | rhs;
    }

    void borf(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] | ram_[rhs];
    }

    void bandl(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] & rhs;
    }
    
    void bandf(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] & ram_[rhs];
    }
    
    void bxorl(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] ^ rhs;
    }
    
    void bxorf(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] ^ ram_[rhs];
    }

    void bnot(reg_type dest, reg_type src, reg_type){
        ram_[dest] = ~ram_[src];
    }

    void f_not(reg_type dest, reg_type src, reg_type){
        ram_[dest] = !ram_[src];
    }
    
    void f_or(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] || ram_[rhs];
    }
    
    void f_and(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] && ram_[rhs];
    }
    
    void f_xor(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] != ram_[rhs];
    }
    
    void gtl(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] > rhs;
    }
    
    void gtf(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] > ram_[rhs];
    }
    
    void eql(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] == rhs;
    }
    
    void eqf(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] == ram_[rhs];
    }
    
    void ltl(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] < rhs;
    }
    
    void ltf(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] < ram_[rhs];
    }
    
    void gtel(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] >= rhs;
    }
    
    void gtef(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] >= ram_[rhs];
    }
    
    void ltel(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] <= rhs;
    }
    
    void ltef(reg_type dest, reg_type lhs, reg_type rhs){
        ram_[dest] = ram_[lhs] <= ram_[rhs];
    }
};


static constexpr const char * mcu_help_string = 
R"---(digital::mcu 0.6.1 2021-09-12

!!!!! FIX ARGS
!!!!! EXPLAIN LABELS
!!!!! ADD INSTRUCTION JUMP IF NOT EQUAL

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
  MOVL:    Move Literal,          move the litreral arg2   to  [arg1]
  MOVF:    Move  File,            move             [arg2]  to  [arg1]
  MOVPF:   Move [File] to  File,  move            [[arg2]] to  [arg1]
  MOVFP:   Move  File  to [File], move             [arg2]  to [[arg1]]
  MOVPP:   Move [File] to [File], move            [[arg2]] to [[arg1]]
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
class mcu_sim_t : public comp::component_t {
public:
    using reg_type = T;
    using gpio_type = gpio_t<reg_type>;
    using adc_type = adc_t<reg_type>;
    
    using mcu_type = microcontroller_t<T>;
    
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_vcc_, error);
        info << utils::find_node_s(nmap, next(args), node_gnd_, error);
        if(error) return info;
        
        size_t rom_size    = next_as<size_t>(args, 0x100);
        size_t ram_size    = next_as<size_t>(args, 0x100);
        source_file_name_  = next(args);
        bool debug_rom     = next_as<bool>(args, false);
        bool debug_op      = next_as<bool>(args, false);
        double frequency   = next_as<double>(args, 1e6);
        instruction_delay_ = next_as<double>(args, 0);
        //gpio_impedance_    = next_as<double>(args, 10);
        
        period_ = 1/frequency;
        
        mcu_ = mcu_type(rom_size, ram_size, debug_op);
        mcu_.read_file(source_file_name_);
        
        if(debug_rom) {
            printf("%s\n", mcu_.dump_rom().c_str());
        }
        
        while(!args.empty()) {
            node::node_t * node_pin = nullptr;
            
            std::string port = next(args);
            
            const std::string id_pin_node = next(args);
            
            info << utils::find_node_s(nmap, id_pin_node, node_pin, error);
            if(error) return info;
            
            if(port.substr(0, 2) == "PA") {
                std::cout << "Adding " << port.substr(0, 2) << "." << port.substr(2) << " (" << port << ") to node " << id_pin_node << "\n";
                gpio_vector_.push_back(
                    gpio_type(
                        node_vcc_,
                        node_gnd_,
                        node_pin,
                        mcu_.ram(mcu_type::PORTA),
                        mcu_.ram(mcu_type::PINA),
                        mcu_.ram(mcu_type::DDRA),
                        reg_type(std::stoull(port.substr(2))),
                        0.8,
                        1.6,
                        15
                    )
                );
                //info << utils::add_printable_s(pmap, label() + ":pin@" + id_pin_node, "", gpio_vector_.back().data());
            }
            else if(port.substr(0, 2) == "PB") {
                gpio_vector_.push_back(
                    gpio_type(
                        node_vcc_,
                        node_gnd_,
                        node_pin,
                        mcu_.ram(mcu_type::PORTB),
                        mcu_.ram(mcu_type::PINB),
                        mcu_.ram(mcu_type::DDRB),
                        reg_type(std::stoull(port.substr(2))),
                        0.8,
                        1.6,
                        15
                    )
                );
                //info << utils::add_printable_s(pmap, label() + ":pin@" + id_pin_node, "", gpio_vector_.back().data());
            }
            else if(port.substr(0, 3) == "ADC"){
                adc_vector_.push_back(
                    adc_type(
                        node_gnd_,
                        node_pin,
                        &mcu_.ram(mcu_.sfr_map[port]),
                        next_as<double>(args)
                    )
                );
            }
        }
        
        if(gpio_vector_.empty()){
            info << "[ERROR] <configure> missing pins\n";
            return info;
        }
        
        return info;
    }
    
    void clear_variables(){
        next_clock_time_ = period_;
        next_clock_diff_ = 0;
        
        mcu_.reset();
    }
    
    //void setup(sim::simulator_t * simulator, const double voltage_difference_max, const double time_step_max) override { }
    
    void update(double time, double /*last_time_step*/) override {
        for(adc_type & adc : adc_vector_){
            adc.update();
        }
        
        if(time >= next_clock_time_){
            next_clock_time_ += period_;
            mcu_.clock();
            using namespace std::chrono_literals;//------------------------------------------------------
            if(instruction_delay_ != 0){
                std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1>>(instruction_delay_));
            }
        }
        
        for(gpio_type & gpio : gpio_vector_){
            gpio.update_node();
        }
        
        next_clock_diff_ = next_clock_time_ - time;
    }
    
    double time_step_max(const double /*time*/) const override {
        return next_clock_diff_;
    }
    
    const char * help() const override {
        return mcu_help_string;
    }
    
private:
    std::vector<gpio_type> gpio_vector_;
    std::vector<adc_type> adc_vector_;
    
    node::node_t * node_vcc_;
    node::node_t * node_gnd_;
    
    mcu_type mcu_;
    
    std::string source_file_name_;
    double period_;
    
    double instruction_delay_;
    
    // sim var
    double next_clock_time_ = 0;
    double next_clock_diff_ = 0;
};

autoreg::reg<mcu_sim_t<uint8_t>> mcu8_reg(database::components(), "digital::mcu8-v2");

} // namespace mcu_v2

