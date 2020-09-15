#include "localSearch/TOP_Helpers.hpp"
#include "localSearch/TOP_Costs.hpp"
#include "localSearch/Moves/Swap.hpp"

#include <fstream>
#include <algorithm>
#include <sstream>
#include <filesystem>
#include <random>
#include <chrono>
#include <vector>
#include <string>

using namespace EasyLocal::Debug;
namespace fs = std::filesystem;

/**
 * Struct that represent Chao's results
 */
struct chaoResults {
  string file;
  double chaoOptimum;
};

int main(int argc, const char* argv[])
{
  // variables
  int errors = 0, cnt_istances =  0;
  string met, line;
  TOP_Input in;
  vector<chaoResults> chaoRes;

  ParameterBox main_parameters("main", "Main Program options");

  // The set of arguments added are the following:
  
  // Parameter<string> instance("instance", "Input instance", main_parameters); 
  Parameter<int> seed("seed", "Random seed", main_parameters);
  Parameter<string> method("method", "Solution method (empty for tester)", main_parameters);   
  Parameter<string> init_state("init_state", "Initial state (to be read from file)", main_parameters);
  Parameter<string> output_file("output_file", "Write the output to a file (filename required)", main_parameters);
 
  // 3rd parameter: false = do not check unregistered parameters
  // 4th parameter: true = silent
  CommandLineParameters::Parse(argc, argv, false, true);  

  // if(!instance.IsSet()) {
  //   cout << "Error: --main::instance filename option must always be set" << endl;
  //   return 1;
  // }

  if(seed.IsSet()) {
    Random::SetSeed(seed);
  }

  //Open and read the file of Chao's results
  ifstream optStream("./paramIn/chaoResults.txt"); 
  if (!optStream) {
    throw runtime_error("  ERROR: Unable to open Chao's Results file");
  }
  
  // Read all the lines into chao's file
  while(std::getline(optStream, line)) {
    std::istringstream iss(line); //Split the input string
    std::vector<string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
    chaoRes.push_back({.file = results[0], .chaoOptimum = stod(results[1])}); //Populate the vector of chao's results
  }
  optStream.close();

  fs::create_directories("solutions");
  if(method == string("SA")) {
    met = string("SA");
  } else if(method == string("HC")) {
    met = string("HC");
  } else if(method == string("TS")) {
    met = string("TS");
  } else { // if (method.GetValue() == string("SD"))
    met = string("SD");
  }
  string titleFile = "solutions/SolLocalSearch#" + met + ".csv";
  ofstream solutionsStream(titleFile);
  if(!solutionsStream) {
    throw runtime_error("  ERROR: Unable to open Solutions file");
  }
  
  for (const auto &file : fs::directory_iterator("./instances")) { //For each instance
    double best = 0;
    if (file.path().extension() != ".txt")
      continue;

    cerr << "Processing: " << file.path().filename() << endl; 
    {
      ifstream is(file.path());
      if (!is) {
        ++errors;
        cerr << "  ERROR: Unable to open Instance file" << endl;
        continue;
      }
      is >> in;
      in.name = file.path().filename().replace_extension("").string();
    }
    TOP_Output out_prec(in);

    {
      ifstream is("./outputs/routeHops/bestRoutes/" + in.name + ".out");
      if (!is) {
        ++errors;
        cerr << "  ERROR: Unable to open bestRoutes Instance file" << endl;
        continue;
      }
      is >> out_prec;
    }

    TOP_StateManager TOP_sm(in);
    
    TOP_CostContainer cc(in); // Create costs
    cc.AddCostComponents(TOP_sm); // Add all cost components

    TOP_MoveSwapNeighborhoodExplorer TOP_nhe(in, TOP_sm, cc); // Create and add delta costs

    TOP_OutputManager TOP_om(in);
    
    // runners
    HillClimbing<TOP_Input, TOP_State, TOP_MoveSwap> TOP_hc(in, TOP_sm, TOP_nhe, "TOP_MoveHillClimbing");
    SteepestDescent<TOP_Input, TOP_State, TOP_MoveSwap> TOP_sd(in, TOP_sm, TOP_nhe, "TOP_MoveSteepestDescent");
    TabuSearch<TOP_Input, TOP_State, TOP_MoveSwap> TOP_ts(in, TOP_sm, TOP_nhe, "TOP_MoveTabuSearch");
    SimulatedAnnealing<TOP_Input, TOP_State, TOP_MoveSwap> TOP_sa(in, TOP_sm, TOP_nhe, "TOP_MoveSimulatedAnnealing");

    // tester
    Tester<TOP_Input, TOP_Output, TOP_State> tester(in, TOP_sm, TOP_om);
    MoveTester<TOP_Input, TOP_Output, TOP_State, TOP_MoveSwap> swap_move_test(in, TOP_sm, TOP_om, TOP_nhe, "TOP_Move move", tester);

    SimpleLocalSearch<TOP_Input, TOP_Output, TOP_State> TOP_solver(in, TOP_sm, TOP_om, "TOP solver");
    if(!CommandLineParameters::Parse(argc, argv, true, false)) {
      return 1;
    }

    if(method == string("SA")) {
      TOP_solver.SetRunner(TOP_sa);
    } else if(method == string("HC")) {
      TOP_solver.SetRunner(TOP_hc);
    } else if(method == string("TS")) {
      TOP_solver.SetRunner(TOP_ts);
    } else { // if (method.GetValue() == string("SD"))
      TOP_solver.SetRunner(TOP_sd);
    }
      
    auto result = TOP_solver.Solve();
    // result is a tuple: 0: solutio, 1: number of violations, 2: total cost, 3: computing time
    TOP_Output out = result.output;
    if(output_file.IsSet()) { // write the output on the file passed in the command line
      ofstream os(static_cast<string>(output_file).c_str());
      // os << out << endl;
      os << "Cost: " << result.cost.total << endl;
      os << "Time: " << result.running_time << "s " << endl;
      os.close();
    } else { // write the solution in the standard output
      // cout << out << endl;
      cout << "Cost: " << result.cost.total << endl;
      cout << "Time: " << result.running_time << "s " << endl;					
    }

    string titleDir = "outputs/localsearch/" + met;
    fs::create_directories(titleDir);
    if(out.PointProfit() == 0) { // No solution found, the problem is unfeasible
      { // Print the outputs on file
        
        ofstream os(titleDir / file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          cerr << "  ERROR: Unable to open output file" << endl;
          continue;
          }
        os << in << "h 0";
      }
      {
        string titleDir = "outputs/routeHops/localsearch/" + met;
        fs::create_directories(titleDir);
        ofstream os(titleDir/ file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          cerr << "  ERROR: Unable to open output file" << endl;
          continue;
        }
        os << "h 0" << endl;
      }
    } else {
      { // Print the outputs on file
        ofstream os(titleDir / file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          cerr << "  ERROR: Unable to open output file" << endl;
          continue;
        }
        os << in << out;
      }
      {
        string titleDir = "outputs/routeHops/localsearch/" + met;
        fs::create_directories(titleDir);
        ofstream os(titleDir/ file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          cerr << "  ERROR: Unable to open output file" << endl;
          continue;
        }
        os << out << endl;
      }
    }

    // Print a ".csv" file with all the scores
    if(chaoRes[cnt_istances].file == file.path().filename()) { // Compare with Chao
      if(chaoRes[cnt_istances].chaoOptimum == best) {
        solutionsStream << file.path().filename() << "," << chaoRes[cnt_istances].chaoOptimum << "," << best << "," << 1.0 << endl;
        ++cnt_istances;
        continue;
      }
      solutionsStream << file.path().filename() << "," << 
                         chaoRes[cnt_istances].chaoOptimum << "," << 
                         best << "," << 
                         best / chaoRes[cnt_istances].chaoOptimum << endl;
      ++cnt_istances;
    }
    else { // New map found
      solutionsStream << file.path().filename() << "," << best << "," << "(new map)" << endl;
    } 
  }
  solutionsStream.close();
  cerr << "Total errors: " << ((double)errors) / 2 << endl;
  return 0;
}
