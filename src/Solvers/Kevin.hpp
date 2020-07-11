#ifndef SOLVERS_KEVIN_HPP
#define SOLVERS_KEVIN_HPP

#include <random>
#include <nlohmann/json.hpp>
#include "../TOP_Data.hpp"

using namespace nlohmann;

void SolveKevin(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, json& options);

#endif
