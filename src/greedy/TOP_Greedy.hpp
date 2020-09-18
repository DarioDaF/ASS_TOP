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
void GreedySolver(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, double wProfit, double wTime, double maxDeviation, double wNonCost);

/**
 * Solve for one instance, all the partial solution associated and update the best one using default parameter ranges.
 *
 * @param in constant input
 * @param out constant output
 * @param rng seed generator to save the solution and its informations
 * @return [void]
 */
void GreedyRangeSolver(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, std::ostream& log = std::cout);

#endif
