#include "common/TOP_Data.hpp"
#include "greedy/Kevin.hpp"
#include "common/Utils.hpp"

#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <filesystem>
#include <random>
#include <chrono>
#include <vector>
#include <string>

using namespace std;
namespace fs = std::filesystem;

/**
 * Struct that represent Chao's results
 */
struct chaoResults {
    string file;
    double chaoOptimum;
};

/**
 * Struct that represent the input data
 */
struct paramGraphs { // Using a struct for manage the data in input
    double parameter;
    double result;
    double minimum;
    double maximum;
    double devStd;
};

/**
 * MainPlotter.cpp is a main that takes all the instances and solve them with a fixed set of params. The aim of
 * the algorithm is to find the behavior of the one variable parameter from fixing the other parameters. Are removed
 * from the analysis easy and unfeasible instances. From the command line it is mandatory to specify the map on which 
 * it is wanted to perform the analysis (type "0" for all), the start, end and plus for the parameter to analyze and 
 * the fixed values of the other three parameters. All the results are compared to Chao's ones to understan better the 
 * behavoir of the parameter in function of the others.
 * The results are saved into outputs files that could by plots with the python script.
 * 
 * Outputs of the analysis: all the values are expressed in % from the Chao's optimum
 *    Mean profit
 *    Standard Deviation
 *    Mean Profit +/- Standard Deviation
 *    Max Score obtained
 *    Min Score obtained
 * 
 * Input file:
 *    chaoResultsModified.txt : file in which are contained Chao's results modified, in particular those instances that aren't
 *                              unfeasible or that the greedy algorithm resolve as optimal solution.
 *                              The file is located in "parametes_in" directory.
 * 
 *    "instances" files : files that contain the instances to analyze.
 *                        The files are located in "instances" directory. 
 * 
 * Output files:
 *    SolPar[param].csv : file in which are saved the results of the analysis.
 *                        The file is located in "outputs/greedy/plots/Map[param]" or "outputs/greedy/plots" directory.
 * 
 * Usage with also python script: 
 *    Example of command line:
 *      ./ *.exe [map] [paramToTest] [from] [to] [increment] [otherParam1] [otherParam2] [otherParam3] ...
 *    
 *    Usage:
 *      [map] -> 0 to select all the maps
 *      if [paramToTest] is "maxDeviation" , then [otherparam1] is "wProfit", [otherParam2] is "wTime", [otherParam3] is "wNonCost" ... 
 *
 *    Specific example: solve all instance, variable param wProfit from 0 to 3.1 plus 0.1, other param fixed to 1.0 
 *      ./MainTest.exe 0 wProfit 0.0 3.1 0.1 1.0 1.0 1.0 
 * 
 * Plot with python script the variation of parameter:
 *    Usage:
 *      python3 ./plots/plotParams.py [map] [parameter]
 * 
 *    Example: plot all the maps with param wProfit
 *      python3 ./plotter/plotParams.py 0 wProfit
 *      
 * @param argc number of items in the command line
 * @param argv items in the command line
 * @return resolve all the instances and print outputs and parameters in files
 */
int main(int argc, char *argv[]) {

  // Seed generator
  random_device rd; // Can be not random...
  auto millis = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count();
  mt19937::result_type seed =
    (mt19937::result_type)rd() ^
    (mt19937::result_type)millis;
  mt19937 rng(seed);

  // Variables
  vector<paramGraphs> sol;
  vector<chaoResults> chao;
  vector<double> res;
  int errors = 0;
  double cnt_istances = 0.0;
  TOP_Input in;
  string line;

  //Open and read the file of Chao's results
  ifstream optStream("./paramIn/chaoResultsModified.txt"); 
  if (!optStream) {
    throw runtime_error("  ERROR: Unable to open Input file");
  }

  // Read all the lines into chao's file
  while(getline(optStream, line)) {
    std::istringstream iss(line); //Split the input string
    std::vector<string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

    if(*argv[1] == '0') { // All the maps toghether
      if(stoi(results[1]) == 0) { // Remove the input file with 0 of profit
        continue;
      }
    }
    else { // One specified map
      string file = results[0]; // To select only one map
      char ch = file[1];
      if(ch == *argv[1]) {
        if(stoi(results[1]) == 0) { // Remove the input file with 0 of profit
          continue;
        }
      }
      else {
        continue;
      }
    }
    chao.push_back({.file = results[0], .chaoOptimum = stod(results[1])}); //Populate the vector of chao's results
    ++cnt_istances;
  }
  optStream.close();
    
  string path; //Chose the right path based on argc and argv
  if(*argv[1] == '0') { // All maps
    fs::create_directories("outputs/greedy/plots");
    path = "./outputs/greedy/plots/solPar" + (string)argv[2] + ".csv";
  }
  else { // One specified map
    fs::create_directories("outputs/greedy/plots/Map" + (string)argv[1]);
    path = "./outputs/greedy/plots/Map" + (string)argv[1] + "/solPar" + (string)argv[2] + ".csv";
  } 
    
  ofstream solStream(path);
  if (!solStream) {
    throw runtime_error("   ERROR: Unable to open Ouput file");
  }

  string init = (string)argv[3];
  string finish = (string)argv[4];
  string plus = (string)argv[5];
  string argvparam1 = (string)argv[6];
  string argvparam2 = (string)argv[7];
  string argvparam3 = (string)argv[8];

  double from = stod(init);
  double to = stod(finish);
  double over = stod(plus);
  double param1 = stod(argvparam1);
  double param2 = stod(argvparam2);
  double param3 = stod(argvparam3);
  
  if(*argv[1] == '0') { // All maps
    std::cerr << "LOG: Processing All maps" << endl;
  }
  else { // One specified map
    std::cerr << "LOG: Processing map  " << *argv[1] << endl;
  }  

  for(double paramChose = from; paramChose <= to; paramChose += over) { // For every parameter
    
    std::cerr << "LOG: Processing param <" << (string)argv[2] << ": " << paramChose << "> ";

    if((string)argv[2] == "wProfit") {
      cerr << "<wTime: " << param1 << ", maxDeviation: " << param2 << ", wNonCost: " << param3 << ">" << endl;
    }
    else if ((string)argv[2] == "wTime") {
      cerr << "<wProfit: " << param1 << ", maxDeviation: " << param2 << ", wNonCost: " << param3 << ">" << endl;
    }
    else if((string)argv[2] == "maxDeviation") {
      cerr << "<wProfit: " << param1 << ", wTime: " << param2 << ", wNonCost: " << param3 << ">" << endl;
    }
    else if((string)argv[2] == "wNonCost") {
      cerr << "<wProfit: " << param1 << ", wTime: " << param2 << ", maxDeviation: " << param3 << ">" << endl;
    }

    res.clear();
    for(idx_t idx = 0; idx < chao.size(); ++idx) { // For every file in chao vector
      //cout << "Processing: " << chao[idx].file << endl;
      {
        ifstream is("./Instances/" + chao[idx].file);
        if (!is) {
          ++errors;
          throw runtime_error("  ERROR: Unable to open Instance file");
          // continue;
        }
        is >> in;
      }

      TOP_Output out(in); // Solve the greedy and print the results for python plot 
      out.Clear();

      if((string)argv[2] == "wProfit") {
        SolverAll(in, out, rng, paramChose, param1, param2, param3);
      }
      else if ((string)argv[2] == "wTime") {
        SolverAll(in, out, rng, param1, paramChose, param2, param3);
      }
      else if((string)argv[2] == "maxDeviation") {
        SolverAll(in, out, rng, param1, param2, paramChose, param3);
      }
      else if((string)argv[2] == "wNonCost") {
        SolverAll(in, out, rng, param1, param2, param3, paramChose);
      }

      res.push_back(out.PointProfit() / chao[idx].chaoOptimum * 100); //Normalized the solution found
    }

    double sumSol = 0.0;
    //Manage the vector of sol for each parameter
    for(int i = 0; i < res.size(); i++) { //mean profit
      sumSol += res[i];
    }
    sumSol = sumSol / cnt_istances;

    double dstd = 0.0, den = 0.0;
    for(int j = 0; j < res.size(); j++) { //std
      den += (res[j] - sumSol) * (res[j] - sumSol);
    }
    dstd = sqrt(den / cnt_istances);

    double minElem = *min_element(res.begin(), res.end()); //min and max 
    double maxElem = *max_element(res.begin(), res.end());
        
    //cerr <<  maxDeviation << " " << sumSol << " " << dstd << " " << minElem << " " << maxElem << endl;
    sol.push_back({.parameter = paramChose, .result = sumSol, .minimum = minElem, .maximum = maxElem, .devStd = dstd});
  }
  
  for(int idx = 0; idx < sol.size(); ++idx) {
    solStream << sol[idx].parameter << "," << sol[idx].result << "," << sol[idx].minimum << "," << sol[idx].maximum << "," << sol[idx].devStd << endl;
  }
  solStream.close();
  std::cerr << "Total errors: " << ((double)errors) / 2 << endl;
  return 0;
}
