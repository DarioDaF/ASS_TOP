#include "SolverLocal.hpp"

/*
#include <filesystem>
namespace fs = std::filesystem;
*/

#include "../localSearch/TOP_Helpers.hpp"
#include "../localSearch/TOP_Costs.hpp"
#include "../localSearch/Moves/Swap.hpp"

#include <mutex>

#define LOCAL_SETPARAM(param) lsh.SetParameter(#param, param.get(options))

std::mutex _OverallParametrizedMutex; // Global lock for static OverallParametrized array

/**
 * Class to create a thread safe version of EasyLocal solver for TOP problem with runner T 
 * To mantain thread safety use SetParameter only between the construction and Solve, for multiple 
 * runs recreate the object (creation of T seems to be required AFTER parametrized clear!).
 * 
 * To avoid unnecessary lock time put only SetParameter instructions between creation and Solve.
 */
template<typename T>
class LocalSolverHelper {
  private:
    const TOP_Input& in;

  protected:
    TOP_StateManager* sm;
    TOP_CostContainer* cc;
    TOP_MoveSwapNeighborhoodExplorer* nhe;
    TOP_OutputManager* om;
    T* runner;
    SimpleLocalSearch<TOP_Input, TOP_Output, TOP_State>* solver;

    std::unique_lock<std::mutex> _lock { _OverallParametrizedMutex, std::defer_lock }; // Automatically released on destruction

  public:

    LocalSolverHelper(const TOP_Input& in, std::string name, std::string solverName) : in(in) {
      sm = new TOP_StateManager(in);
      cc = new TOP_CostContainer(in); // Create costs
      cc->AddCostComponents(*sm); // Add all cost components
      nhe = new TOP_MoveSwapNeighborhoodExplorer(in, *sm, *cc); // Create and add delta costs
      om = new TOP_OutputManager(in);
  
      _lock.lock();
      CommandLineParameters::Parametrized::OverallParametrized().clear(); // Fix to avoid double registration of destroyed objects notifiers?

      // runners
      runner = new T(in, *sm, *nhe, name);
      solver = new SimpleLocalSearch<TOP_Input, TOP_Output, TOP_State>(in, *sm, *om, solverName);
      solver->SetRunner(*runner);
      
      runner->RegisterParameters(); // Needed to use SetParameter without CommandLineParameters::Parse
    }

    template<typename U>
    void SetParameter(std::string name, const U& value) {
      runner->SetParameter(name, value);
    }

    ~LocalSolverHelper() {
      delete solver;
      delete runner;
      delete om;
      delete nhe;
      delete cc;
      delete sm;
    }

    void Solve(TOP_Output& out, std::ostream& log) {
      _lock.unlock();

      TOP_Output out_prec(in);
      {
        ifstream is("outputs/routeHops/bestRoutes/GB/" + in.name + ".out");
        if (!is) {
          log << "<span style='color: red;'>ERROR: Unable to open bestRoutes Instance file, run from empty solution</span>" << endl;
        } else {
          is >> out_prec;
        }
      }
      double precProfit = out_prec.PointProfit();
      auto result = solver->Resolve(out_prec);
      out = result.output;

      if(result.cost.violations > 0) {
        log << "Violations: " << result.cost.violations << endl;
      } else {
        log << "Delta Profit: "
            << (out.PointProfit() / precProfit - 1) * 100
            << "% in Time: " << result.running_time << "s" << endl;
      }
    }

};

// Solvers

void WebSolverLocalSA::Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) {
  LocalSolverHelper<SimulatedAnnealing<TOP_Input, TOP_State, TOP_MoveSwap>> lsh(in, "TOP_MoveSimulatedAnnealing", "TOP_SA");

  LOCAL_SETPARAM(max_evaluations);
  LOCAL_SETPARAM(cooling_rate);
  LOCAL_SETPARAM(min_temperature);
  LOCAL_SETPARAM(neighbors_sampled);
  LOCAL_SETPARAM(neighbors_accepted);
  LOCAL_SETPARAM(compute_start_temperature);
  LOCAL_SETPARAM(start_temperature);

  lsh.Solve(out, log);
}

void WebSolverLocalHC::Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) {
  LocalSolverHelper<HillClimbing<TOP_Input, TOP_State, TOP_MoveSwap>> lsh(in, "TOP_MoveHillClimbing", "TOP_HC");

  LOCAL_SETPARAM(max_evaluations);
  LOCAL_SETPARAM(max_idle_iterations);

  lsh.Solve(out, log);
}

void WebSolverLocalTS::Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) {
  LocalSolverHelper<TabuSearch<TOP_Input, TOP_State, TOP_MoveSwap>> lsh(in, "TOP_MoveTabuSearch", "TOP_TS");

  LOCAL_SETPARAM(max_evaluations);
  LOCAL_SETPARAM(max_idle_iterations);
  LOCAL_SETPARAM(max_tenure);
  LOCAL_SETPARAM(min_tenure);

  lsh.Solve(out, log);
}

void WebSolverLocalSD::Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) {
  LocalSolverHelper<SteepestDescent<TOP_Input, TOP_State, TOP_MoveSwap>> lsh(in, "TOP_MoveSimulatedAnnealing", "TOP_SA");

  LOCAL_SETPARAM(max_evaluations);

  lsh.Solve(out, log);
}
