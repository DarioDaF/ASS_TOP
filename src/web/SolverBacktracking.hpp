#ifndef WEB_SOLVER_BACKTRACKING_HPP
#define WEB_SOLVER_BACKTRACKING_HPP

#include "Solver.hpp"

#include "../backTracking/TOP_Backtracking.hpp"

class WebSolverBackTracking : public AbstractWebSolver {
  private:
    Web::RParameter<double> wTime { "wTime", "Weight of Time", 0.7, 0.1, 3.4 };
    Web::RParameter<double> wNonCost { "wNonCost", "Weight of Missing Costs", 0, 0, 2 };
    Web::RParameter<double> maxDev { "maxDev", "Maximum detour distence to capture point", 1.5, 0, 6 };
    Web::RParameter<double> maxTime { "maxTime", "Maximum time allowed", 15, 1, 3*60 };
    Web::RParameter<double> nonGreedyDrop { "nonGreedyDrop", "Drop for non-greedy nodes", 0, 0, 1000 };

  protected:
    std::vector<Web::Parameter*> GetParameters() override {
      return { &wTime, &wNonCost, &maxDev, &maxTime, &nonGreedyDrop };
    }

  public:
    std::string name() override { return "Backtrack"; }

    void Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) override {
      TOP_Walker tw(in, 1, wTime.get(options), maxDev.get(options), wNonCost.get(options), nonGreedyDrop.get(options));
      TOP_Checker ck;
      Backtrack(tw, ck, maxTime.get(options));
      const auto& best = ck.GetBest(); // Stripping const make a copy necessary
      if(best.HasOutput()) {
        out = best.Output();
      }
    }

};

#endif
