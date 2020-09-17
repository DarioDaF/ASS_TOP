#include "TOP_Helpers.hpp"
#include "TOP_Costs.hpp"
#include "Moves/Swap.hpp"

#include <fstream>
#include <filesystem>
#include <stdio.h>
#include <string.h>

using namespace EasyLocal::Debug;

#ifdef WITH_JSON_OPTIONS

#include <nlohmann/json.hpp>

/**
 * Solve the problem with the MoveSimulatedAnnealing algorithm from LocalSearch and it's used in the Web Viewer. 
 *
 * @param in constant input
 * @param out constant output
 * @param rng seed generator to save the solution and its informations
 * @param options the file json in which are written the parameters for the web viewer
 * @return [void]
 */
void SolveLocalSA(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options) {
  // Exctract the parameters from the json file. Default values are provided
  TOP_StateManager TOP_sm(in);
  TOP_CostContainer cc(in); // Create costs
  cc.AddCostComponents(TOP_sm); // Add all cost components
  TOP_MoveSwapNeighborhoodExplorer TOP_nhe(in, TOP_sm, cc); // Create and add delta costs
  TOP_OutputManager TOP_om(in);

  CommandLineParameters::Parametrized::OverallParametrized().clear(); // Fix to avoid double registration of destroyed objects notifiers?

  // runners
  SimulatedAnnealing<TOP_Input, TOP_State, TOP_MoveSwap> TOP_sa(in, TOP_sm, TOP_nhe, "TOP_MoveSimulatedAnnealing");
  SimpleLocalSearch<TOP_Input, TOP_Output, TOP_State> TOP_solver(in, TOP_sm, TOP_om, "TOP_SA");
  TOP_solver.SetRunner(TOP_sa);

  TOP_sa.RegisterParameters();
  TOP_sa.SetParameter("compute_start_temperature", json_get_or_default<bool>(options["m"], false));
  TOP_sa.SetParameter("cooling_rate", json_get_or_default<double>(options["n"], (double)0.0001));
  TOP_sa.SetParameter("max_evaluations", json_get_or_default<unsigned long int>(options["o"], (unsigned long int)10000));
  TOP_sa.SetParameter("min_temperature", json_get_or_default<double>(options["p"], (double)0.001));
  TOP_sa.SetParameter("neighbors_accepted", json_get_or_default<unsigned int>(options["q"], (unsigned int)1000));
  TOP_sa.SetParameter("neighbors_sampled", json_get_or_default<unsigned int>(options["r"], (unsigned int)1000));
  TOP_sa.SetParameter("start_temperature", json_get_or_default<double>(options["s"], (double)0.0001));

  TOP_Output out_prec(in);
  bool findInstance = true;
  auto name = in.name.substr(0, 6);
  {
    ifstream is("outputs/routeHops/bestRoutes/" + name + ".out");
    if (!is) {
      cerr << "  ERROR: Unable to open bestRoutes Instance file, run from empty solution" << endl;
      findInstance = false;
    }
    if(findInstance) {
      is >> out_prec;
    }
  }
  double precProfit = out_prec.PointProfit();
  auto result = TOP_solver.Resolve(out_prec);
  out = result.output;
  // Print the output into the shell
  std::cout << "deltaProfit: " 
            << (((100 * out.PointProfit()) / precProfit) - 100) 
            << "% in Time: " << result.running_time << "s " << std::endl;
}

/**
 * Solve the problem with the MoveHillClimbing algorithm from LocalSearch and it's used in the Web Viewer. 
 *
 * @param in constant input
 * @param out constant output
 * @param rng seed generator to save the solution and its informations
 * @param options the file json in which are written the parameters for the web viewer
 * @return [void]
 */
void SolveLocalHC(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options) {
  // Exctract the parameters from the json file. Default values are provided
  TOP_StateManager TOP_sm(in);
  TOP_CostContainer cc(in); // Create costs
  cc.AddCostComponents(TOP_sm); // Add all cost components
  TOP_MoveSwapNeighborhoodExplorer TOP_nhe(in, TOP_sm, cc); // Create and add delta costs
  TOP_OutputManager TOP_om(in);
  
  CommandLineParameters::Parametrized::OverallParametrized().clear(); // Fix to avoid double registration of destroyed objects notifiers?

  // runners
  HillClimbing<TOP_Input, TOP_State, TOP_MoveSwap> TOP_hc(in, TOP_sm, TOP_nhe, "TOP_MoveHillClimbing");
  SimpleLocalSearch<TOP_Input, TOP_Output, TOP_State> TOP_solver(in, TOP_sm, TOP_om, "TOP_HC");
  TOP_solver.SetRunner(TOP_hc);

  // params
  TOP_hc.RegisterParameters();
  TOP_hc.SetParameter("max_evaluations", json_get_or_default<unsigned long int>(options["t"], (unsigned long int)10000));
  TOP_hc.SetParameter("max_idle_iterations", json_get_or_default<unsigned long int>(options["u"], (unsigned long int)100));

  TOP_Output out_prec(in);
  bool findInstance = true;
  auto name = in.name.substr(0, 6);
  {
    ifstream is("outputs/routeHops/bestRoutes/" + name + ".out");
    if (!is) {
      cerr << "  ERROR: Unable to open bestRoutes Instance file, run from empty solution" << endl;
      findInstance = false;
    }
    if(findInstance) {
      is >> out_prec;
    }
  }
  double precProfit = out_prec.PointProfit();
  auto result = TOP_solver.Resolve(out_prec);
  out = result.output;
  // Print the output into the shell
  std::cout << "deltaProfit: " 
            << (((100 * out.PointProfit()) / precProfit) - 100) 
            << "% in Time: " << result.running_time << "s " << std::endl;
}

/**
 * Solve the problem with the MoveTabuSearch algorithm from LocalSearch and it's used in the Web Viewer. 
 *
 * @param in constant input
 * @param out constant output
 * @param rng seed generator to save the solution and its informations
 * @param options the file json in which are written the parameters for the web viewer
 * @return [void]
 */
void SolveLocalTS(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options) {
  // Exctract the parameters from the json file. Default values are provided
  TOP_StateManager TOP_sm(in);
  TOP_CostContainer cc(in); // Create costs
  cc.AddCostComponents(TOP_sm); // Add all cost components
  TOP_MoveSwapNeighborhoodExplorer TOP_nhe(in, TOP_sm, cc); // Create and add delta costs
  TOP_OutputManager TOP_om(in);
  
  CommandLineParameters::Parametrized::OverallParametrized().clear(); // Fix to avoid double registration of destroyed objects notifiers?

  // runners
  TabuSearch<TOP_Input, TOP_State, TOP_MoveSwap> TOP_ts(in, TOP_sm, TOP_nhe, "TOP_MoveTabuSearch");
  SimpleLocalSearch<TOP_Input, TOP_Output, TOP_State> TOP_solver(in, TOP_sm, TOP_om, "TOP_TS");
  TOP_solver.SetRunner(TOP_ts);

  // params
  TOP_ts.RegisterParameters();
  TOP_ts.SetParameter("max_evaluations", json_get_or_default<unsigned long int>(options["v"], (unsigned long int)10000));
  TOP_ts.SetParameter("max_idle_iterations", json_get_or_default<unsigned long int>(options["y"], (unsigned long int)100));
  TOP_ts.SetParameter("max_tenure", json_get_or_default<unsigned int>(options["x"], (unsigned int)20));
  TOP_ts.SetParameter("min_tenure", json_get_or_default<unsigned int>(options["w"], (unsigned int)10));

  TOP_Output out_prec(in);
  bool findInstance = true;
  auto name = in.name.substr(0, 6);
  {
    ifstream is("outputs/routeHops/bestRoutes/" + name + ".out");
    if (!is) {
      cerr << "  ERROR: Unable to open bestRoutes Instance file, run from empty solution" << endl;
      findInstance = false;
    }
    if(findInstance) {
      is >> out_prec;
    }
  }
  double precProfit = out_prec.PointProfit();
  auto result = TOP_solver.Resolve(out_prec);
  out = result.output;
  // Print the output into the shell
  std::cout << "deltaProfit: " 
            << (((100 * out.PointProfit()) / precProfit) - 100) 
            << "% in Time: " << result.running_time << "s " << std::endl;
}

/**
 * Solve the problem with the MoveSteepestDescent algorithm from LocalSearch and it's used in the Web Viewer. 
 *
 * @param in constant input
 * @param out constant output
 * @param rng seed generator to save the solution and its informations
 * @param options the file json in which are written the parameters for the web viewer
 * @return [void]
 */
void SolveLocalSD(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options) {
  // Exctract the parameters from the json file. Default values are provided
  TOP_StateManager TOP_sm(in);
  TOP_CostContainer cc(in); // Create costs
  cc.AddCostComponents(TOP_sm); // Add all cost components
  TOP_MoveSwapNeighborhoodExplorer TOP_nhe(in, TOP_sm, cc); // Create and add delta costs
  TOP_OutputManager TOP_om(in);

  CommandLineParameters::Parametrized::OverallParametrized().clear(); // Fix to avoid double registration of destroyed objects notifiers?

  // runners
  SteepestDescent<TOP_Input, TOP_State, TOP_MoveSwap> TOP_sd(in, TOP_sm, TOP_nhe, "TOP_MoveSteepestDescent");
  SimpleLocalSearch<TOP_Input, TOP_Output, TOP_State> TOP_solver(in, TOP_sm, TOP_om, "TOP_SD");
  TOP_solver.SetRunner(TOP_sd);

  // params
  TOP_sd.RegisterParameters();
  TOP_sd.SetParameter("max_evaluations", json_get_or_default<unsigned long int>(options["z"], (unsigned long int)10000));

  TOP_Output out_prec(in);
  bool findInstance = true;
  auto name = in.name.substr(0, 6);
  {
    ifstream is("outputs/routeHops/bestRoutes/" + name + ".out");
    if (!is) {
      cerr << "  ERROR: Unable to open bestRoutes Instance file, run from empty solution" << endl;
      findInstance = false;
    }
    if(findInstance) {
      is >> out_prec;
    }
  }
  double precProfit = out_prec.PointProfit();
  auto result = TOP_solver.Resolve(out_prec);
  out = result.output;
  // Print the output into the shell
  std::cout << "deltaProfit: " 
            << (((100 * out.PointProfit()) / precProfit) - 100) 
            << "% in Time: " << result.running_time << "s " << std::endl;
}

#endif

