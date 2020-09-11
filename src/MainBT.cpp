#include "backTracking/TOP_Backtracking.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <algorithm>
#include <vector>

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
 * MainBT.cpp is a main that takes all the instances and solve them with the backtracking algorithm. Bacause
 * of the long time that takes the backtracking, it is possible to set a maxTime limit that it takes to resolve 
 * each instance (default value 3 minutes). The main perform a metaheuristic backtracking because it use the greedy 
 * algorithm (with its parameter maxDeviation) plus some other solutions built to solve the TOP problem.  
 * the solution are compared with Chao's ones to evaluate the performance and the resoluts of the backtracking algorithm. 
 * All the outputs are saved in different files to sum up the information and the outputs from which it is obtained 
 * the best optimum.
 * 
 * Input file:
 *    paramTimeBt.txt : file in which are contained tthe max time permitted to execute the backtracking algorithm
 *                      on each instance, expressed in second.
 *                      Mandatory info: time for each instance. If not provided, default at 3 minutes.
 *                      The file is located in "parametes_in" directory.
 * 
 *    chaoResults.txt : file in which are contained Chao's results, used to compare greedy scores whith Chao's ones.
 *                      The file is located in "parametes_in" directory.
 * 
 *    "instances" files : files that contain the instances to solve.
 *                        The files are located in "instances" directory. 
 * 
 * Output files:
 *    SolBacktracking.csv : file in which it is saved for each instance the algorithm results and the comparison whith chao's one.
 *                          The file is located in "solutions" directory.
 *    
 *    "outputs/backtracking" files : for all the instances, it is saved a file which contain the input and the output in standard 
 *                                   form. Some useful information as the path, the hop and the score obtained are provided.
 *                                   The file are located in "outputs/backtracking" directory.
 * 
 *    "outputs/routeHops/backtarcking" files : for all the instances it is saved the solution obtained if hops form to read and use
 *                                             it in the resolution of other algortims (i.e. Local Search).
 *                                             The files are located in "outputs/hops/greedy" directory.
 *    
 * @param argc number of items in the command line
 * @param argv items in the command line
 * @return resolve all the instances and print outputs and parameters in files
 */
int main(int argc, char* argv[]) {

  double maxTime = 3.0 * 60.0; //  Default Time limit: 3 minutes
  int errors = 0, cnt_istances =  0;
  bool timeDefault = false;
  vector<chaoResults> chaoRes;
  string line;
  

  //Open the file conteining the max time for execute the program for each instance
  ifstream paramTime("./paramIn/paramTimeBt.txt"); 
  if (!paramTime) {
    cerr << "  ERROR: Unable to open MaxTime file" << endl;
    timeDefault = true;
  }

  if(!timeDefault) {
    while(getline(paramTime, line)) {
      std::istringstream iss(line); //Split the input string
      std::vector<string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
      if (results[0] == "timeMax") {
        maxTime = stod(results[1]);
      }
      else {
        continue;
      }
    }
    paramTime.close();
  }
  cout << "LOG: timeMax limit set to " << maxTime << "s" << endl; 

  //Open and read the file of Chao's results
  ifstream optStream("./paramIn/chaoResults.txt"); 
  if (!optStream) {
    throw runtime_error("   ERROR: Unable to open Chao's file");
  }

  // Read all the lines into chao's file
  while(getline(optStream, line)) {
    std::istringstream iss(line); //Split the input string
    std::vector<string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
    chaoRes.push_back({.file = results[0], .chaoOptimum = stod(results[1])}); //Populate the vector of chao's results
  }
  optStream.close();

  //Open and write the file of results for each instance
  fs::create_directories("solutions");
  ofstream solutionsStream("solutions/SolBacktracking.csv");
  if(!solutionsStream) {
    throw runtime_error("  ERROR: Unable to open Solution file");
  }
  solutionsStream << "timeMax limit set to " << maxTime << "s" << endl;

  for (const auto &file : fs::directory_iterator("./instances")) { //For each instance
    TOP_Input in;
    string line;
    bool foundParam = true;

    // Default weight parameters
    double maxDeviation = 1.5; // Max deviation admitted: 0.3 for insertion point with min deviation from real path
    double wProfit = 0; 
    double wTime = 0;
    double wNonCost = 0;

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
    
    //Open the file conteining the param maxDeviation
    ifstream paramStream("./outputs/greedy/paramGreedy/" + in.name + ".out"); 
    if (!paramStream) {
      std::cerr << "  ERROR: Unable to open Parameter file or not found" << endl;
      foundParam = false;
    }

    if (foundParam) { // If found param, set it, otherwise use default value
      while(getline(paramStream, line)) {
        std::istringstream iss(line); //Split the input string
        std::vector<string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

        if (results[2] == "null") { // Default parameters
          break;
        }
        if (results[0] == "Profit") { // Skip first line
          continue;
        }

        if(results[1] == "A:") {
          wProfit = stod(results[2]);
        }
        else if(results[1] == "B:") {
          wTime = stod(results[2]);
        }
        else if(results[1] == "C:") {
          maxDeviation = stod(results[2]);
        }
        else if(results[1] == "D:") {
          wNonCost = stod(results[2]);
        }  
      }
      paramStream.close();
    }
    
    TOP_Walker tw(in);
    TOP_Checker ck;
    //Backtrack(tw, ck, maxTime, wProfit, wTime, maxDeviation, wNonCost);
    
    { // Print the output
      fs::create_directories("outputs/backtracking");
      std::ofstream os("outputs/backtracking" / file.path().filename().replace_extension(".out"));
      if (!os) {
        ++errors;
        std::cerr << "  ERROR: Unable to open output file" << std::endl;
        continue;
      }
      if (ck.GetBestCost() == 0) {
        os << in;
        os << "h 0";
      }
      else {
        os << in << ck.GetBest();
      }
    }
    {
      fs::create_directories("outputs/routeHops/backtracking");
      std::ofstream os("outputs/routeHops/backtracking" / file.path().filename().replace_extension(".out"));
      if (!os) {
        ++errors;
        std::cerr << "  ERROR: Unable to open output file" << std::endl;
        continue;
      }
      if (ck.GetBestCost() == 0) {
        os << in;
        os << "h 0";
      }
      else {
        os << ck.GetBest();
      }
    }

    // Print a ".csv" file with all the scores
    if(chaoRes[cnt_istances].file == file.path().filename()) { // Compare with Chao
      if(chaoRes[cnt_istances].chaoOptimum == -ck.GetBestCost()) {
        solutionsStream << file.path().filename() << ";" << chaoRes[cnt_istances].chaoOptimum << ";" << -ck.GetBestCost() << ";" << 1.0 << endl;
        ++cnt_istances;
        continue;
      }
      solutionsStream << file.path().filename() << ";" << chaoRes[cnt_istances].chaoOptimum << ";" << -ck.GetBestCost() << ";0," << static_cast<int>(-ck.GetBestCost() / chaoRes[cnt_istances].chaoOptimum *10000) << endl;
      ++cnt_istances;
    }
    else { // New map found
      solutionsStream << file.path().filename() << ";" << -ck.GetBestCost() << ";" << "(new map)" << endl;
    } 
  }
  solutionsStream.close();
  return 0;
}
