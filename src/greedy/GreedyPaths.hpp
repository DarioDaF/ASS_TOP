#ifndef GREEDY_PATHS_HPP
#define GREEDY_PATHS_HPP

#include <string>

inline std::string GetGreedyBestParamsPath(const std::string& instName) {
  return "outputs/GreedyBestParams/" + instName + ".params";
}

#endif
