#include "cli.hpp"

cli_t::cli_t(){
    log("<cli_t> mapping commands");
    cmdtab_["help"]      = &cli_t::help;
    cmdtab_["end"]       = &cli_t::end;
    cmdtab_["run"]       = &cli_t::run;
    cmdtab_["component"] = &cli_t::component;
    cmdtab_["node"]      = &cli_t::node;
    cmdtab_["print"]     = &cli_t::print;
    cmdtab_["param"]     = &cli_t::param;
    cmdtab_["edit"]      = &cli_t::edit;
    cmdtab_["show"]      = &cli_t::show;
    cmdtab_["read"]      = &cli_t::read_file_cmd;
    cmdtab_["bind"]      = &cli_t::bind;
    log("<cli_t> commands mapped");
}
