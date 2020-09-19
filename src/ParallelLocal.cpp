#include <iostream>
#include <fstream>

#include <vector>
#include <map>
#include <set>

#include <thread>
#include <shared_mutex>
#include <ctpl_stl.h>

#include <nlohmann/json.hpp>
#include <filesystem>

namespace fs = std::filesystem;

#include "common/Utils.hpp"

#include "common/TOP_Data.hpp"
#include "web/SolverLocal.hpp"

using namespace std;

void runThread(int id, const TOP_Input& in, lockflush& lf, std::string algo, std::string desc, nlohmann::json& options) {
  auto log = lf.get();

  random_device rd; // Can be not random...
  auto millis = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count();
  mt19937::result_type seed =
    (mt19937::result_type)rd() ^
    (mt19937::result_type)millis;
  mt19937 rng(seed);

  TOP_Output out(in);

  nullstream nullStream;
  if(algo == "SD") {
    WebSolverLocalSD sd;
    sd.Solve(in, out, rng, options, nullStream);
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
  
  ofstream ofs("outputs/test.out");
  lockflush lf(ofs);

  cout << "Starting pool" << endl;
  for(const auto& [name, in] : ins) {
    for(const auto& algo : { "SD", "HC" }) {
      pool.push(runThread, ref(in), ref(lf), algo, "default", nlohmann::json {});
    }
  }

  cout << "Waiting on pool" << endl;
  int i = 0;
  do {
    this_thread::sleep_for(chrono::seconds(1));
    cout << ".";
    if(++i > 10) {
      i = 0;
      cout << endl << "Tasks queued: " << pool.queued() << endl;
    }
  } while(pool.n_idle() < pool.size());
  cerr << endl;
  pool.stop(true); // Wait for solution

  return 0;
}
