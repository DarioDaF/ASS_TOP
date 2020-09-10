#ifndef SOLVERS_KEVIN_HPP
#define SOLVERS_KEVIN_HPP

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
 * @param a weight that multiplies the first (profit) factor of the rating equation
 * @param b weight that multiplies the second (travel time) factor of the rating equation
 * @param c max deviation admitted on the path of the car
 * @param d weight that multiplies the third (no choosing cost or losses) factor of the rating equation
 * @return [void]
 */
void SolverAll(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, double a, double b, double c, double d);

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
void SolveKevin(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options) {
  // Exctract the parameters from the json file. Default value: 1
  double a = json_get_or_default<float>(options["a"], 1);
  double b = json_get_or_default<float>(options["b"], 1);
  double c = json_get_or_default<float>(options["c"], 1);
  double d = json_get_or_default<float>(options["d"], 1);
  SolverAll(in, out, rng, a, b, c, d);
}

#endif

#endif
