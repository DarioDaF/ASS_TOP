#include "localSearch/TOP_Helpers.hpp"
#include "localSearch/TOP_Costs.hpp"
#include "localSearch/Moves/Swap.hpp"

using namespace EasyLocal::Debug;

int main(int argc, const char* argv[])
{
  ParameterBox main_parameters("main", "Main Program options");

  // The set of arguments added are the following:
  
  Parameter<string> instance("instance", "Input instance", main_parameters); 
  Parameter<int> seed("seed", "Random seed", main_parameters);
  Parameter<string> method("method", "Solution method (empty for tester)", main_parameters);   
  Parameter<string> init_state("init_state", "Initial state (to be read from file)", main_parameters);
  Parameter<string> output_file("output_file", "Write the output to a file (filename required)", main_parameters);
 
  // 3rd parameter: false = do not check unregistered parameters
  // 4th parameter: true = silent
  CommandLineParameters::Parse(argc, argv, false, true);  

  if (!instance.IsSet())
    {
      cout << "Error: --main::instance filename option must always be set" << endl;
      return 1;
    }
  TOP_Input in(instance);

  if (seed.IsSet())
    Random::SetSeed(seed);
  
  TOP_StateManager TOP_sm(in);
  
  TOP_CostContainer cc(in); // Create costs
  cc.AddCostComponents(TOP_sm); // Add all cost components

  TOP_MoveSwapNeighborhoodExplorer TOP_nhe(in, TOP_sm, cc); // Create and add delta costs

  TOP_OutputManager TOP_om(in);
  
  // runners
  HillClimbing<TOP_Input, TOP_State, TOP_MoveSwap> TOP_hc(in, TOP_sm, TOP_nhe, "TOP_MoveHillClimbing");
  SteepestDescent<TOP_Input, TOP_State, TOP_MoveSwap> TOP_sd(in, TOP_sm, TOP_nhe, "TOP_MoveSteepestDescent");
  SimulatedAnnealing<TOP_Input, TOP_State, TOP_MoveSwap> TOP_sa(in, TOP_sm, TOP_nhe, "TOP_MoveSimulatedAnnealing");

  // tester
  Tester<TOP_Input, TOP_Output, TOP_State> tester(in,TOP_sm,TOP_om);
  MoveTester<TOP_Input, TOP_Output, TOP_State, TOP_MoveSwap> swap_move_test(in,TOP_sm,TOP_om,TOP_nhe, "TOP_Move move", tester); 

  SimpleLocalSearch<TOP_Input, TOP_Output, TOP_State> TOP_solver(in, TOP_sm, TOP_om, "TOP solver");
  if (!CommandLineParameters::Parse(argc, argv, true, false))
    return 1;

  if (!method.IsSet())
    { // If no search method is set -> enter in the tester
      if (init_state.IsSet())
	    tester.RunMainMenu(init_state);
      else
	    tester.RunMainMenu();
    }
  else
    {

      if (method == string("SA"))
        {
          TOP_solver.SetRunner(TOP_sa);
        }
      else if (method == string("HC"))
        {
          TOP_solver.SetRunner(TOP_hc);
        }
      else // if (method.GetValue() == string("SD"))
        {
          TOP_solver.SetRunner(TOP_sd);
        }
      auto result = TOP_solver.Solve();
	  // result is a tuple: 0: solutio, 1: number of violations, 2: total cost, 3: computing time
      TOP_Output out = result.output;
      if (output_file.IsSet())
        { // write the output on the file passed in the command line
          ofstream os(static_cast<string>(output_file).c_str());
          os << out << endl;
          os << "Cost: " << result.cost.total << endl;
					os << "Time: " << result.running_time << "s " << endl;
          os.close();
        }
      else
        { // write the solution in the standard output
          cout << out << endl;
          cout << "Cost: " << result.cost.total << endl;
					cout << "Time: " << result.running_time << "s " << endl;					
        }
   }
  return 0;
}
