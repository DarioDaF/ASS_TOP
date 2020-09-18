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

/**
 * MainLocalSearch.cpp is a main that takes all the instances and solve them with the LocalSearch's algorithms.
 * From command line, it takes the name of the algorithm that run on all the instances. The parametrs for all the 
 * algorithms are set by deafualt inside the main. To emiliorate the results receive from the greedy and the backtraking, 
 * this main run the choosen algorithm on all the best routeHops received from the two previous algorithms.
 * All the outputs are saved in different files to sum up the information and the outputs from which it is obtained 
 * the best optimum.
 * 
 * Input file:
 * 
 *    chaoResults.txt : file in which are contained Chao's results, used to compare greedy scores whith Chao's ones.
 *                      The file is located in "parametes_in" directory.
 * 
 *    "instances" files : files that contain the instances to solve.
 *                        The files are located in "instances" directory. 
 *  
 *    "outputs/routeHops/bestRoutes" files : files that contains the best route given by the greedy and backtracking algorthms.
 * 
 * Output files:
 *    SolLocalSearch[alg].csv : file in which it is saved for each instance the algorithm results and the comparison whith chao's one.
 *                              The file is located in "solutions" directory.
 *    
 *    "outputs/localsearch/[alg]" files : for all the instances, it is saved a file which contain the input and the output in standard 
 *                                        form. Some useful information as the path, the hop and the score obtained are provided.
 *                                        The file are located in "outputs/localsearch/[alg]" directory.
 * 
 *    "outputs/routeHops/localsearch/[alg]" files : for all the instances it is saved the solution obtained if hops form to read and use
 *                                                  it in the resolution of other algortims (i.e. Local Search).
 *                                                  The files are located in "outputs/routeHops/localsearch/[alg]" directory.
 * 
 * Usage:
 *    ./MainLocalSearch.exe [algorithm] [route]
 *    - algorithms : 
 *                SA : MoveSimulatedAnnealing
 *                HC : MoveHillClimbing
 *                TS : MoveTabuSearch
 *                SD : MoveSteepestDescent
 *    - route :
 *                GB : greedy or backtracking
 *                LS : localsearch
 * 
 * @param argc number of items in the command line
 * @param argv items in the command line
 * @return resolve all the instances and print outputs and parameters in files
 */
int main(int argc, const char* argv[])
{
  // variables
  int errors = 0, cnt_istances =  0;
  string met, line, method, routeB;
  TOP_Input in;
  vector<chaoResults> chaoRes;

  if(argc < 2) {
    throw runtime_error("  ERROR: missing Algorithm [SA, HC, TS, SD] and Routes [GB, LS]");
  }
  method = argv[1];
  routeB = argv[2];

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
  string titleFile = "solutions/SolLocalSearch" + routeB + "#" + met + ".csv";
  ofstream solutionsStream(titleFile);
  if(!solutionsStream) {
    throw runtime_error("  ERROR: Unable to open Solutions file");
  }
  
  for (const auto &file : fs::directory_iterator("./instances")) { //For each instance
    if (file.path().extension() != ".txt")
      continue;

    std::cerr << "Processing: " << file.path().filename() << std::endl; 
    {
      ifstream is(file.path());
      if (!is) {
        ++errors;
        std::cerr << "  ERROR: Unable to open Instance file" << std::endl;
        continue;
      }
      is >> in;
      in.name = file.path().filename().replace_extension("").string();
    }
    
    TOP_Output out_prec(in);
    {
      ifstream is("./outputs/routeHops/bestRoutes/" + routeB + "/" + in.name + ".out");
      if (!is) {
        ++errors;
        std::cerr << "  ERROR: Unable to open bestRoutes Instance file" << std::endl;
        continue;
      }
      is >> out_prec;
    }
    double precProfit = out_prec.PointProfit();

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

    SimpleLocalSearch<TOP_Input, TOP_Output, TOP_State> TOP_solver(in, TOP_sm, TOP_om, "TOP solver");

    if(method == string("SA")) {
      TOP_solver.SetRunner(TOP_sa);
    } else if(method == string("HC")) {
      TOP_solver.SetRunner(TOP_hc);
    } else if(method == string("TS")) {
      TOP_solver.SetRunner(TOP_ts);
    } else { // if (method.GetValue() == string("SD"))
      TOP_solver.SetRunner(TOP_sd);
    }

    // TOP_ts.RegisterParameters();
    // std::cout << TOP_ts.ParametersToJSON() << endl; 

    // TOP_sa.RegisterParameters();
    // std::cout << TOP_sa.ParametersToJSON() << endl;

    // TOP_hc.RegisterParameters();
    // std::cout << TOP_hc.ParametersToJSON() << endl;

    // TOP_sd.RegisterParameters();
    // std::cout << TOP_sd.ParametersToJSON() << endl;

    if(method == string("SA")) {
      TOP_sa.RegisterParameters();
      TOP_sa.SetParameter("compute_start_temperature", (bool)true);
      TOP_sa.SetParameter("cooling_rate", (double) 1e-7);
      TOP_sa.SetParameter("max_evaluations", (unsigned long int)std::numeric_limits<unsigned long int>::max());
      TOP_sa.SetParameter("min_temperature", (double)0.000001);
      TOP_sa.SetParameter("neighbors_accepted", (unsigned int)100000);
      TOP_sa.SetParameter("neighbors_sampled", (unsigned int)10000000);
      TOP_sa.SetParameter("start_temperature", (double)1000);
    } 
    else if(method == string("HC")) {
      TOP_hc.RegisterParameters();
      TOP_hc.SetParameter("max_evaluations", (unsigned long int)std::numeric_limits<unsigned long int>::max());
      TOP_hc.SetParameter("max_idle_iterations", (unsigned long int)1000000);
    } 
    else if(method == string("TS")) {
      TOP_ts.RegisterParameters();
      TOP_ts.SetParameter("max_evaluations", (unsigned long int)std::numeric_limits<unsigned long int>::max());
      TOP_ts.SetParameter("max_idle_iterations", (unsigned long int)10000);
      TOP_ts.SetParameter("max_tenure", (unsigned int)50);
      TOP_ts.SetParameter("min_tenure", (unsigned int)10);
    } 
    else { // if (method.GetValue() == string("SD"))
      TOP_sd.RegisterParameters();
      TOP_sd.SetParameter("max_evaluations", (unsigned long int)std::numeric_limits<unsigned long int>::max());
    }

    auto result = TOP_solver.Resolve(out_prec);
    TOP_Output out = result.output;
    
    // Print the output into the shell
    if(result.cost.violations == 0) {
      if(precProfit != 0) {
        std::cout << "Cost: " << -result.cost.total 
                              << " [From:" << out_prec.PointProfit() 
                              << ", deltaProfit: " 
                              << (((100 * out.PointProfit()) / precProfit) - 100)  << "%]" << std::endl;
      }
      else {
        std::cout << "Cost: " << -result.cost.total 
                              << " [From:" << out_prec.PointProfit() 
                              << ", deltaProfit: " 
                              << 0.0  << "%]" << std::endl;        
      }
    }
    else {
      std::cout << "Cost: " << precProfit
                            << " [From:" << precProfit
                            << ", deltaProfit: " 
                            << 0.0  << "%]" << std::endl;        
    }
    
    std::cout << "Time: " << result.running_time << "s " << std::endl;	
    				
    // Print the outputs on file in different format
    string titleDir = "outputs/localsearch/" + routeB + "/" + met;
    fs::create_directories(titleDir);
    if(result.cost.violations > 0) { // No solution found, the problem is unfeasible
      {
        
        ofstream os(titleDir / file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          std::cerr << "  ERROR: Unable to open output file" << std::endl;
          continue;
        }
        os << in << "h 0";
      }
      {
        string titleDir = "outputs/routeHops/localsearch/"+  routeB + "/" + met;
        fs::create_directories(titleDir);
        ofstream os(titleDir/ file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          std::cerr << "  ERROR: Unable to open output file" << std::endl;
          continue;
        }
        os << "h 0" << endl;
      }
    } else {
      { // Print the outputs on file
        ofstream os(titleDir / file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          std::cerr << "  ERROR: Unable to open output file" << std::endl;
          continue;
        }
        os << in << out;
      }
      {
        string titleDir = "outputs/routeHops/localsearch/" + routeB + "/" + met;
        fs::create_directories(titleDir);
        ofstream os(titleDir/ file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          std::cerr << "  ERROR: Unable to open output file" << std::endl;
          continue;
        }
        os << out << endl;
      }
    }

    // Print a ".csv" file with all the scores
    if(chaoRes[cnt_istances].file == file.path().filename()) { // Compare with Chao
      if(chaoRes[cnt_istances].chaoOptimum == out.PointProfit()) {
        solutionsStream << file.path().filename() 
                        << "," << chaoRes[cnt_istances].chaoOptimum 
                        << "," << out.PointProfit() 
                        << "," << 1.0 << std::endl;
        ++cnt_istances;
        continue;
      }
      else if(result.cost.violations > 0) {
        if(precProfit == 0) {
          solutionsStream << file.path().filename() << "," 
                          << chaoRes[cnt_istances].chaoOptimum << "," 
                          << precProfit << "," 
                          << 1 << std::endl;
          ++cnt_istances;
          continue;
        }
        else {
          solutionsStream << file.path().filename() << "," 
                          << chaoRes[cnt_istances].chaoOptimum << "," 
                          << precProfit << "," 
                          << precProfit / chaoRes[cnt_istances].chaoOptimum << std::endl;
          ++cnt_istances;
          continue;
        }
      }
      else {
        solutionsStream << file.path().filename() << "," 
                        << chaoRes[cnt_istances].chaoOptimum << "," 
                        << out.PointProfit() << "," 
                        << out.PointProfit() / chaoRes[cnt_istances].chaoOptimum << std::endl;
        ++cnt_istances;
        continue;
      }  
    }
    else { // New map found
      solutionsStream << file.path().filename() << "," << out.PointProfit() << "," << "(new map)" << std::endl;
    } 
  }
  solutionsStream.close();
  std::cerr << "Total errors: " << ((double)errors) / 2 << std::endl;
  return 0;
}
