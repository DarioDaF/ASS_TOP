#ifndef WEB_SOLVER_LOCAL_HPP
#define WEB_SOLVER_LOCAL_HPP

#include "Solver.hpp"

class WebSolverLocalSA : public AbstractWebSolver {
  private:
    // Possible parameters:
    //   Web::TParameter<bool>
    //   Web::RParameter<double, int, long, float, ...>
    // WARNING: Numbers too near the limits create problem on double cast (int -> number(js) -> int)
    Web::RParameter<unsigned long int> max_evaluations { "max_evaluations", "Max Evaluations", std::numeric_limits<long int>::max(), 0, std::numeric_limits<long int>::max() };
    Web::RParameter<double> cooling_rate { "cooling_rate", "Cooling Rate", 1e-7, 0, 1 };
    Web::RParameter<double> min_temperature { "min_temperature", "Min Temperature", 1e-6, 0, 1000 };
    Web::RParameter<unsigned int> neighbors_sampled { "neighbors_sampled", "N# Sampled Neighbors", 10000000, 0, 10000000 };
    Web::RParameter<unsigned int> neighbors_accepted { "neighbors_accepted", "N# Accepted Neighbors", 100000, 0, 10000000 };
    Web::TParameter<bool> compute_start_temperature { "compute_start_temperature", "Compute Starting Temperature", true };
    Web::RParameter<double> start_temperature { "start_temperature", "StartTemperature", 100, 0, 1000 };

  protected:
    std::vector<Web::Parameter*> GetParameters() override {
      return { &max_evaluations, &cooling_rate, &min_temperature, &neighbors_sampled, &neighbors_accepted, &compute_start_temperature, &start_temperature };
    }

  public:
    std::string name() override { return "LocalSA"; }

    void Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) override;
};

class WebSolverLocalHC : public AbstractWebSolver {
  private:
    Web::RParameter<unsigned long int> max_evaluations { "max_evaluations", "Max Evaluations", std::numeric_limits<long int>::max(), 0, std::numeric_limits<long int>::max() };
    Web::RParameter<unsigned long int> max_idle_iterations { "max_idle_iterations", "Max Idle Iterations", 1000000, 0, 100000000 };

  protected:
    std::vector<Web::Parameter*> GetParameters() override {
      return { &max_evaluations, &max_idle_iterations };
    }

  public:
    std::string name() override { return "LocalHC"; }

    void Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) override;
};

class WebSolverLocalTS : public AbstractWebSolver {
  private:
    Web::RParameter<unsigned long int> max_evaluations { "max_evaluations", "Max Evaluations", std::numeric_limits<long int>::max(), 0, std::numeric_limits<long int>::max() };
    Web::RParameter<unsigned long int> max_idle_iterations { "max_idle_iterations", "Max Idle Iterations", 10000, 0, 100000000 };
    Web::RParameter<unsigned int> max_tenure { "max_tenure", "Maximum steps to remember tabu", 50, 0, 1000 };
    Web::RParameter<unsigned int> min_tenure { "min_tenure", "Minimum steps to remember tabu", 20, 0, 1000 };

  protected:
    std::vector<Web::Parameter*> GetParameters() override {
      return { &max_evaluations, &max_idle_iterations, &max_tenure, &min_tenure };
    }

  public:
    std::string name() override { return "LocalTS"; }

    void Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) override;
};

class WebSolverLocalSD : public AbstractWebSolver {
  private:
    Web::RParameter<unsigned long int> max_evaluations { "max_evaluations", "Max Evaluations", std::numeric_limits<long int>::max(), 0, std::numeric_limits<long int>::max() };

  protected:
    std::vector<Web::Parameter*> GetParameters() override {
      return { &max_evaluations };
    }

  public:
    std::string name() override { return "LocalSD"; }

    void Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) override;
};

#endif
