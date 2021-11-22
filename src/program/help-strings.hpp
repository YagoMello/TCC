#ifndef HELP_STRINGS_HPP
#define HELP_STRINGS_HPP

namespace help {

static constexpr const char * program = 
R"---(Welcome to the simulator!

Created by Yago Teodoro de Mello, 2021
Version: Alpha 0.1 2021-09-06

This program was developed as a different way to simulate circuits, using 
capacitive nodes instead of solving linear equations.

The program accepts the following commands:
- component
- node
- run
- end
- print
- edit
- show
- bind
- param
- read

Comments are started with "#".

To learn more about each command, type "help command".
)---";

static constexpr const char * component = 
R"---(The component command inserts a new component into the simulator.
Ths syntax is:
  component identifier model args...

The identifier is the name the simulator will use to differentiate this component from the others.
The model determines the type of the component, is's equations and behaviour. To see every available model, type "show models".
The arguments are defined by the model. To see them, type "help component $MODEL" where $MODEL is the model.
)---";

static constexpr const char * node = 
R"---(The node command inserts a new node into the simulator.
The syntax is:
  node identifier capacitance

The identifier is the name the simulator will use to differentiate this node from the others.
The capacitance should be a value greater than 0. 
)---";

static constexpr const char * run = 
R"---(The run command starts the simulation.
The syntax is:
  run

Remember to add printers to the simulation!
Otherwise, there will be no output from the simulation.
To abort the simulation, press "Ctrl + C".
At the beginning of the simulation, component and node data are cleared.
)---";

static constexpr const char * end = 
R"---(The end command closes the simulator.
The syntax is:
  end

Before closing the simulator, the buffers are flushed and the destructors called.
)---";

static constexpr const char * print = 
R"---(The print command controls the way the simulation objects should print their data.
The syntax is:
  print category args...

This command accepts the following categories:
- var

var:
  The var category contains the variables ("printables") from the components.
  To enable a printable, type:
    print var identifier true
  And to disable:
    print var identifier false
  
  To show the available printables, type "show printables".
  
  To create a printable in a component, include the utils lib "src/libs/utils.hpp", then:
    utils::add_printable(printable_map, label, dimension, var_to_print);
  
  To print a current from a var named current_, in the .configure function, use:
    bool is_ok = utils::add_printable(pmap, this->label() + ":i", "A", current_);
    if(!is_ok) return {is_ok, "Failed to add printable"};
)---";

static constexpr const char * edit = 
R"---(The edit command modifies parameters from the components and nodes.
The syntax is:
  edit category args...

The categories are:
- component
- node

component:
  The component category expects the following syntax:
    edit component identifier args...
  
  The configuration function of the component will be called with the args. This is the only
  way to modify components after they are created.

node:
  The node category expects the following syntax:
    edit node identifier property
  
  The properties are:
  - gnd
  - capacitande
  - hide
  
  gnd:
    Accepts "true" or "false". If set to true, the node will behave as if it has
    infinte capacitance.
  
  capacitance:
    Accepts a value. The capacitance will be set to the new value.
  
  hide:
    Accepts "true" or "false". If set to true, the node data will not appear in 
    program output.
)---";

static constexpr const char * show = 
R"---(The show command displays the content of various categories.
The syntax is:
  show category

The categories are:
- models
- components
- nodes
- printables
- shared-vars

models:
  Displays the component models.

components:
  Displays the components currently in the simulator and their identifiers.

nodes:
  Displays the nodes, their identifiers, capacitances and status.

printables:
  Displays the printables created by the componentes inserted in the simulator, and if they are active.

shared-vars:
  Display the shared vars in the simulator. To bind shared vars from different components, use the "bind" command.
)---";

static constexpr const char * bind = 
R"---(The bind command connects variables from different components.
This can be used to conect a motor shaft to a wheel, a mosfet to a heatsink, a data bus to a component.
The data type can be anything due to type erasure, allowing great flexibility.
The syntax is:
  bind var1 var2 

The existing shared vars can be displayed with the command "show shared-vars".
)---";

static constexpr const char * param = 
R"---(The param command modifies key parameters of the simulation.
The syntax is:
  param parameter-name value

The parameters are:
- vd-max
- ts-max
- duration

vd-max:
  Maximum voltage change in each node at the end of the iteration.
  This value can seriously impact the simulation performance.
  A value too big will lead to bad simulation accuracy, and a value
  too small will slow down the simulation.
  Values in the range of 10uV to 200uV show the best mix of accuracy and performance.

ts-max:
  Maximum time step duration.
  This values limits the step duration if the voltage change in the nodes is too small, which
  would result in a long time step.
  Values in the range of 10ns to 2000ns show the best mix of accuracy and performance.

duration:
  The simulation duration.
  The maximum value the "time" variable can have before the simulation is terminated.
)---";

static constexpr const char * read = 
R"---(The read command reads the commands in a file. It is used to automate the data input.
Ths syntax is:
  read file-name.extension

The file can ba a plain text file.
This command is called when the simulator executable is invoked with args:
  ./simulator some-file-with-commands.txt
)---";

}

#endif // HELP_STRINGS_HPP