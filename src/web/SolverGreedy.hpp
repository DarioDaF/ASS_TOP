#ifndef WEB_SOLVER_GREEDY_HPP
#define WEB_SOLVER_GREEDY_HPP

#include "Solver.hpp"

#include "../greedy/TOP_Greedy.hpp"

class WebSolverGreedy : public AbstractWebSolver {
  private:
    Web::RParameter<double> wTime { "wTime", "Weight of Time", 0.7, 0.1, 3.4 };
    Web::RParameter<double> wNonCost { "wNonCost", "Weight of Missing Costs", 0, 0, 2 };
    Web::RParameter<double> maxDev { "maxDev", "Maximum detour distence to capture point", 1.5, 0, 6 };

  protected:
    std::vector<Web::Parameter*> GetParameters() override {
      return { &wTime, &wNonCost, &maxDev };
    }

  public:
    std::string name() override { return "Greedy Single"; }

    void Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) override {
      GreedySolver(in, out, rng, 1, wTime.get(options), maxDev.get(options), wNonCost.get(options));
    }

};

class WebSolverGreedyRange : public AbstractWebSolver {
  public:
    std::string name() override { return "Greedy Range"; }

    void Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) override {
      GreedyRangeSolver(in, out, rng, log);
    }

};

#endif
