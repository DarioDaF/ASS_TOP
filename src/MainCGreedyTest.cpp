#include <iostream>
#include <fstream>

#include <vector>
#include <algorithm>

#include <thread>
#include <ctpl_stl.h>

#include "common/TOP_Data.hpp"
#include "greedy/TOP_Greedy.hpp"

using namespace std;

//#define N_ITERS 10000
//#define N_ITERS (600*600)
#define N_ITERS 10000

#define MDEV_MIN 0.0
#define MDEV_MAX 6.0

void runThread(int id, TOP_Output& out, double maxDeviation) {
  //cerr << "START Thread: " << id << " @ " << maxDeviation << endl;

  random_device rd; // Can be not random...
  auto millis = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count();
  mt19937::result_type seed =
    (mt19937::result_type)rd() ^
    (mt19937::result_type)millis;
  mt19937 rng(seed);

  SolverAll(out.in, out, rng, 1.1, 0.7, maxDeviation, 0.0);
  //cerr << "END Thread: " << id << " @ " << maxDeviation << " -> " << out.PointProfit() << endl;
}

int main() {
  auto nHWThreads = thread::hardware_concurrency();

  cout << "HW Threads: " << nHWThreads << endl;
  
  cout << "Initializing thread pool" << endl;
  ctpl::thread_pool pool(nHWThreads);

  cout << "Reading input" << endl;
  TOP_Input in;
  {
    ifstream ifs("instances/p5.4.q.txt");
    if(!ifs) {
      throw new runtime_error("Unable to open file");
    }
    ifs >> in;
  }
  
  cout << "Preparing outputs" << endl;
  //vector<TOP_Output> outs(N_ITERS);
  //transform(outs.begin(), outs.end(), outs.begin(), [&in] () { return TOP_Output(in); });
  vector<TOP_Output> outs;
  for(int i = 0; i < N_ITERS; ++i) {
    outs.emplace_back(in);
  }

  double increment = (MDEV_MAX - MDEV_MIN) / (N_ITERS - 1);
  double maxValue = MDEV_MIN + (N_ITERS - 1) * increment;

  cout << "Min: " << MDEV_MIN << " | Inc: " << increment << " | Max: " << maxValue << endl;

  cout << "Starting pool" << endl;
  for(int i = 0; i < N_ITERS; ++i) {
    double maxDev = MDEV_MIN + i * increment;
    pool.push(runThread, ref(outs[i]), maxDev);
  }

  cout << "Waiting on pool" << endl;
  pool.stop(true); // Wait for solution

  cout << "Solutions:" << endl;
  int prevProfit = 0;
  for(int i = 0; i < N_ITERS; ++i) {
    int currProfit = outs[i].PointProfit();
    double maxDev = MDEV_MIN + i * increment;
    if(prevProfit != currProfit) {
      // Print
      cout << maxDev << " -> " << currProfit << endl;
      prevProfit = currProfit;
    }
  }
}
