// INCLUDE <>
#include <iostream>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"
#include "../libs/shared_var.hpp"

// sauce https://itunesu-assets.itunes.apple.com/itunes-assets/CobaltPublic69/v4/91/74/5b/91745b70-3e9e-3df6-13e1-184999dd13ef/333-6766082688398459779-DSSI_2.pdf?a=v%3D3%26artistId%3D1280875126%26podcastId%3D1053000847%26podcastName%3D%25E5%258B%2595%25E6%2585%258B%25E7%25B3%25BB%25E7%25B5%25B1%25E6%25A8%25A1%25E6%2593%25AC%25E8%2588%2587%25E5%25AF%25A6%25E7%258F%25BE%2BDynamic%2BSystem%2BSimulation%2Band%2BImplementation%26episodeId%3D1000355707184%26episodeName%3D%2528%25E8%25AC%259B%25E7%25BE%25A9%2529%2BDSSI%2B2%25EF%25BC%259AModeling%2Bof%2BDC%2Bmotor%26episodeKind%3Dpdf%26pageLocation%3Ditc

class dc_motor2_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list & ls, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        ra_ = next_as<double>(args, 0.5);
        la_ = next_as<double>(args, 1.5e-3);
        kt_ = next_as<double>(args, 0.05);
        jm_ = next_as<double>(args, 2.5e-4);
        te_ = next_as<double>(args, 0);
        bm_ = next_as<double>(args, 0);
        
        stau_.create(ls, label() + ":tau");
        sjm_.create(ls, label() + ":J");
        sw_.create(ls, label() + ":w");
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", i_);
        info << utils::add_printable_s(pmap, label() + ":w", "rad/s", w_);
        
        return info;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        stau_.init();
        sjm_.init();
        sw_.init();
        
        sjm_ += jm_;
    }
    
    void clear_variables() override {
        w_  = 0;
        i_  = 0;
        di_ = 0;
        sw_ = 0;
        dw_ = 0;
    }
    
    void update(double, double last_time_step) override {
        i_ += di_ * last_time_step;
        
        node_positive_->current_out(i_);
        node_negative_->current_in(i_);
        
        double vd = voltage_difference(node_positive_, node_negative_);
        
        di_ = (-ra_*i_ - kt_*sw_ + vd ) / la_;
        stau_ += kt_*i_ - bm_*sw_ - te_;
    }
    
    void post_update(double, double time_step) override {
        dw_ = stau_ / sjm_;
        stau_ = 0;
        sw_ += dw_ * time_step;
        w_   = sw_;
    }
    
private:
    // sim var
    double i_  = 0;
    double di_ = 0;
    double dw_ = 0;
    
    // shared var
    shared::var<double> stau_;
    shared::var<double> sjm_;
    shared::var<double> sw_;
    
    // print helper
    double w_;
    
    // params
    double ra_ = 0.5;
    double la_ = 1.5e-3;
    double kt_ = 0.05;
    double jm_ = 2.5e-4;
    double te_ = 0;
    double bm_ = 0;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<dc_motor2_t> dc_motor2_reg(database::components(), "dc-motor-2");


