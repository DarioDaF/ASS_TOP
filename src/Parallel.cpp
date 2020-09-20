#include <iostream>
#include <fstream>

#include <vector>
#include <map>
#include <set>

#include <algorithm>

#include <thread>
#include <shared_mutex>
#include <ctpl_stl.h>

#include <nlohmann/json.hpp>

#include <filesystem>

namespace fs = std::filesystem;

#include <regex>

#include "common/Utils.hpp"
#include "common/JsonUtils.hpp"

#include "common/TOP_Data.hpp"
#include "web/SolverLocal.hpp"
#include "web/SolverGreedy.hpp"
#include "web/SolverBacktracking.hpp"

using namespace std;

void runThread(int id, const TOP_Input& in, lockflush& lf, std::string algo, std::string desc, nlohmann::json options) {
  auto log = lf.get();

  random_device rd; // Can be not random...
  auto millis = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count();
  mt19937::result_type seed =
    (mt19937::result_type)rd() ^
    (mt19937::result_type)millis;
  mt19937 rng(seed);

  TOP_Output out(in);

  nullstream nullStream;
  transform(algo.begin(), algo.end(), algo.begin(), ::toupper);
  if(algo == "GREEDY") {
    WebSolverGreedy greedy;
    greedy.Solve(in, out, rng, options, nullStream);
  } else if(algo == "GREEDY RANGE") {
    WebSolverGreedyRange greedyRange;
    greedyRange.Solve(in, out, rng, options, nullStream);
  } else if(algo == "BT") {
    WebSolverBackTracking bt;
    bt.Solve(in, out, rng, options, nullStream);
  } else if(algo == "SD") {
    WebSolverLocalSD sd;
    sd.Solve(in, out, rng, options, nullStream);
  } else if(algo == "TS") {
    WebSolverLocalTS ts;
    ts.Solve(in, out, rng, options, nullStream);
  } else if(algo == "SA") {
    WebSolverLocalSA sa;
    sa.Solve(in, out, rng, options, nullStream);
  } else if(algo == "HC") {
    WebSolverLocalHC hc;
    hc.Solve(in, out, rng, options, nullStream);
  }
  log << in.name << "," << algo << "," << desc << "," <<  out.PointProfit() << "," << out.Feasible() << endl;
  log.flush();
}

int main() {
  auto nHWThreads = thread::hardware_concurrency();

  cout << "HW Threads: " << nHWThreads << endl;
  
  cout << "Initializing thread pool" << endl;
  ctpl::thread_pool pool(nHWThreads);
  //ctpl::thread_pool pool(std::max(1U, nHWThreads - 1));

  cout << "Reading input" << endl;

  map<string, TOP_Input> ins;
  for(const auto &file : fs::directory_iterator("./instances")) { //For each instance
    if(file.path().extension() != ".txt")
      continue;
    auto name = file.path().filename().replace_extension("").string();
    ins[name] = TOP_Input {};
    ins[name].name = name;
    {
      ifstream ifs(file.path());
      if(!ifs) {
        throw new runtime_error("Unable to open file");
      }
      ifs >> ins[name];
    }
  }

  ofstream ofs("outputs/parallel.csv");
  lockflush lf(ofs);

  cout << "Starting pool" << endl;
  int tasks = 0;
  {
    nlohmann::json config;
    {
      ifstream ifs("paramIn/parallel.json");
      if(!ifs) {
        throw runtime_error("Unable to open configuration file");
      }
      ifs >> config;
    }

    for(const auto& action : config) {
      regex namesRegex(action["names"]);
      for(const auto& [name, in] : ins) {
        if(regex_match(name, namesRegex)) {
          for(const auto& algo : action["algos"]) {
            pool.push(runThread, ref(in), ref(lf),
              algo["type"],
              json_get_or_default(algo, "descr", std::string {}),
              json_get_or_default(algo, "options", nlohmann::json::object())
            );
            ++tasks;
          }
        }
      }
    }
    // Descope config to free RAM
  }
  cout << "Queued " << tasks << " tasks" << endl;

  cout << "Waiting on pool" << endl;
  int i = 0;
  do {
    this_thread::sleep_for(chrono::seconds(1));
    cout << ".";
    cout.flush();
    if(++i > 10) {
      i = 0;
      int q = pool.queued();
      cout << endl << "Tasks completed: " << 100 * (1 - ((double)q) / tasks) << "% (missing: " << q << ")" << endl;
    }
  } while(pool.n_idle() < pool.size());
  cerr << endl;
  pool.stop(true); // Wait for solution

  return 0;
}
