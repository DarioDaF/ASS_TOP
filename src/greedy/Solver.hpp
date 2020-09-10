#ifndef SOLVERS_SOLVER_HPP
#define SOLVERS_SOLVER_HPP

#include <string>
#include <random>
#include <nlohmann/json.hpp>
#include "../common/TOP_Data.hpp"

using namespace nlohmann;

typedef void (*SolveFunction)(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, json& options);
struct SolverEntry_s {
  SolveFunction fn;
  std::string name;
};

#endif
