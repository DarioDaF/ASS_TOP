#ifndef WEB_SOLVER_BACKTRACKING_HPP
#define WEB_SOLVER_BACKTRACKING_HPP

#include "Solver.hpp"

#include "../backTracking/TOP_Backtracking.hpp"
#include "../greedy/GreedyPaths.hpp"

class WebSolverBackTracking : public AbstractWebSolver {
  private:
    Web::RParameter<double> wTime { "wTime", "Weight of Time", DEF_WTIME, 0.1, 3.4 };
    Web::RParameter<double> wNonCost { "wNonCost", "Weight of Missing Costs", DEF_WNONCOST, 0, 2 };
    Web::RParameter<double> maxDev { "maxDev", "Maximum detour distence to capture point", DEF_MAXDEV, 0, 6 };
    Web::RParameter<double> maxTime { "maxTime", "Maximum time allowed", 15, 1, 3*60 };
    Web::RParameter<double> nonGreedyDrop { "nonGreedyDrop", "Drop for non-greedy nodes", 0, 0, 1000 };

  protected:
    std::vector<Web::Parameter*> GetParameters() override {
      return { &wTime, &wNonCost, &maxDev, &maxTime, &nonGreedyDrop };
    }

  public:
    static constexpr double DEF_WTIME = 0.7;
    static constexpr double DEF_WNONCOST = 0;
    static constexpr double DEF_MAXDEV = 1.5;

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

class WebSolverBackTrackingFocus : public AbstractWebSolver {
  private:
    Web::RParameter<double> maxTime { "maxTime", "Maximum time allowed", 15, 1, 3*60 };
    Web::RParameter<double> nonGreedyDrop { "nonGreedyDrop", "Drop for non-greedy nodes", 0, 0, 1000 };

  protected:
    std::vector<Web::Parameter*> GetParameters() override {
      return { &maxTime, &nonGreedyDrop };
    }

  public:
    std::string name() override { return "Backtrack Focus"; }

    void Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) override {
      double wTime = WebSolverBackTracking::DEF_WTIME;
      double maxDev = WebSolverBackTracking::DEF_MAXDEV;
      double wNonCost = WebSolverBackTracking::DEF_WNONCOST;

      {
        std::ifstream ifsParams(GetGreedyBestParamsPath(in.name));
        if(!ifsParams) {
          log << "<span style='color: red;'>ERROR: Unable to open paramGreedy file, run from default parameters</span>" << std::endl;
        }

        bool wTimeSet = false;
        bool maxDevSet = false;
        bool wNonCostSet = false;

        std::string type, name, val;
        while(true) {
          ifsParams >> type >> name >> val;
          if(ifsParams.eof()) {
            break;
          }
          if(type != "Parameter" && val != "null") {
            if(name == "wTime:") {
              wTime = stod(val);
              wTimeSet = true;
            } else if(name == "maxDeviation:") {
              maxDev = stod(val);
              maxDevSet = true;
            } else if(name == "wNonCost:") {
              wNonCost = stod(val);
              wNonCostSet = true;
            }
          }
        }

        if(!wTimeSet) {
          log << "<span style='color: orange;'>WARNING: Missing wTime or null, using default</span>" << std::endl;
        }
        if(!maxDevSet) {
          log << "<span style='color: orange;'>WARNING: Missing maxDev or null, using default</span>" << std::endl;
        }
        if(!wNonCostSet) {
          log << "<span style='color: orange;'>WARNING: Missing wNonCost or null, using default</span>" << std::endl;
        }
      }

      TOP_Walker tw(in, 1, wTime, maxDev, wNonCost, nonGreedyDrop.get(options));
      TOP_Checker ck;
      Backtrack(tw, ck, maxTime.get(options));
      const auto& best = ck.GetBest(); // Stripping const make a copy necessary
      if(best.HasOutput()) {
        out = best.Output();
      }
    }

};

#endif
