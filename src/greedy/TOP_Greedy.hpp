#ifndef SOLVERS_TOP_Greedy_HPP
#define SOLVERS_TOP_Greedy_HPP

#include "../common/TOP_Data.hpp"
#include <random>

/***************
 * Declaration *
 ***************/

/**
 * Solve for one instance, all the partial solution associated and update the best one.
 *
 * @param in constant input
 * @param out constant output
 * @param rng seed generator to save the solution and its informations
 * @param wProfit weight that multiplies the first (profit) factor of the rating equation
 * @param wTime weight that multiplies the second (travel time) factor of the rating equation
 * @param maxDeviation max deviation admitted on the path of the car
 * @param wNonCost weight that multiplies the third (no choosing cost or losses) factor of the rating equation
 * @return [void]
 */
void SolverAll(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, double wProfit, double wTime, double maxDeviation, double wNonCost);

#ifdef WITH_JSON_OPTIONS

#include <nlohmann/json.hpp>

/**
 * Solve the problem with the greedy algorithm SolverGreedy, it's used in the Web Viewer. 
 *
 * @param in constant input
 * @param out constant output
 * @param rng seed generator to save the solution and its informations
 * @param options the file json in which are written the parameters for the web viewer
 * @return [void]
 */
void SolveGreedy(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options) {
  // Exctract the parameters from the json file. Default values are provided
  // double wProfit = json_get_or_default<float>(options["wProfit"], 1.1);
  // double wTime = json_get_or_default<float>(options["wTime"], 0.7);
  // double maxDeviation = json_get_or_default<float>(options["maxDeviation"], 1.5);
  // double wNonCost = json_get_or_default<float>(options["wNonCost"], 0.0);

  double wProfit = json_get_or_default<float>(options["a"], 1.1);
  double wTime = json_get_or_default<float>(options["b"], 0.7);
  double maxDeviation = json_get_or_default<float>(options["c"], 1.5);
  double wNonCost = json_get_or_default<float>(options["d"], 0.0);
  SolverAll(in, out, rng, wProfit, wTime, maxDeviation, wNonCost);
}

#endif

#endif