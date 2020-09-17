#ifndef WEB_SOLVER_LOCAL_HPP
#define WEB_SOLVER_LOCAL_HPP

#include "Solver.hpp"

#include <filesystem>
namespace fs = std::filesystem;

#include "../localSearch/TOP_Helpers.hpp"
#include "../localSearch/TOP_Costs.hpp"
#include "../localSearch/Moves/Swap.hpp"

#define LOCAL_SETPARAM(param) runner.SetParameter(#param, param.get(options))

class WebSolverLocalSA : public AbstractWebSolver {
  private:
    // Possibili tipi parametri:
    //   Web::TParameter<bool>
    //   Web::RParameter<double, int, long, float, ...>
    Web::TParameter<bool> compute_start_temperature { "compute_start_temperature", "Compute Starting Temperature", false };
    Web::RParameter<double> cooling_rate { "cooling_rate", "Cooling Rate", 1e-8, 0, 1 };
    Web::RParameter<unsigned long int> max_evaluations { "max_evaluations", "Max Evaluations", 100000000, 0, 10000000000 };
    Web::RParameter<double> min_temperature { "min_temperature", "Min Temperature", 1e-5, 0, 1000 };
    Web::RParameter<unsigned int> neighbors_accepted { "neighbors_accepted", "N# Accepted Neighbors", 10000, 0, 10000000 };
    Web::RParameter<unsigned int> neighbors_sampled { "neighbors_sampled", "N# Sampled Neighbors", 1000000, 0, 10000000 };
    Web::RParameter<double> start_temperature { "start_temperature", "StartTemperature", 100, 0, 1000 };

  protected:
    std::vector<Web::Parameter*> GetParameters() override {
      return { &compute_start_temperature, &cooling_rate, &max_evaluations, &min_temperature, &neighbors_accepted, &neighbors_sampled, &start_temperature };
    }

  public:
    std::string name() override { return "LocalSA"; }

    void Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options) override {
      TOP_StateManager TOP_sm(in);
      TOP_CostContainer cc(in); // Create costs
      cc.AddCostComponents(TOP_sm); // Add all cost components
      TOP_MoveSwapNeighborhoodExplorer TOP_nhe(in, TOP_sm, cc); // Create and add delta costs
      TOP_OutputManager TOP_om(in);

      CommandLineParameters::Parametrized::OverallParametrized().clear(); // Fix to avoid double registration of destroyed objects notifiers?

      // runners
      SimulatedAnnealing<TOP_Input, TOP_State, TOP_MoveSwap> runner(in, TOP_sm, TOP_nhe, "TOP_MoveSimulatedAnnealing");
      SimpleLocalSearch<TOP_Input, TOP_Output, TOP_State> TOP_solver(in, TOP_sm, TOP_om, "TOP_SA");
      TOP_solver.SetRunner(runner);

      runner.RegisterParameters();
      LOCAL_SETPARAM(compute_start_temperature);
      LOCAL_SETPARAM(cooling_rate);
      LOCAL_SETPARAM(max_evaluations);
      LOCAL_SETPARAM(min_temperature);
      LOCAL_SETPARAM(neighbors_accepted);
      LOCAL_SETPARAM(neighbors_sampled);
      LOCAL_SETPARAM(start_temperature);

      TOP_Output out_prec(in);
      bool findInstance = true;
      auto name = fs::path(in.name).replace_extension(".out");
      {
        ifstream is("outputs/routeHops/bestRoutes" / name);
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

};

#endif
