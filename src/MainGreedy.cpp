#include "common/TOP_Data.hpp"
#include "greedy/TOP_Greedy.hpp"
#include "common/Utils.hpp"

#include <fstream>
#include <algorithm>
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
 * MainGreedy.cpp is a main that takes all the instances and solve them with a set of params. The results
 * are compared with Chao's ones to evaluate the performance and the resoluts of the greedy algorithm. All
 * the outputs are saved in different files to sum up the information and the outputs from which it is obtained 
 * the best optimum. It is also provided a seed number generator to reverse the algorithm starting from the output.
 * 
 * Input file:
 *    paramSetGr.txt : file in which are contained the range of all params in which the algorith resolve the problem.
 *                     Mandatory info: start, end and plus number for each paramter.
 *                     The file is located in "parametes_in" directory.
 * 
 *    chaoResults.txt : file in which are contained Chao's results, used to compare greedy scores whith Chao's ones.
 *                      The file is located in "parametes_in" directory.
 * 
 *    "instances" files : files that contain the instances to solve.
 *                        The files are located in "instances" directory. 
 * 
 * Output files:
 *    SolGreedy.csv : file in which it is saved the range of parameted and for each instance the algorithm results 
 *                    and the comparison whith chao's one. The file is located in "solutions" directory.
 *    
 *    "outputs/greedy" files : for all the instances, it is saved a file which contain the input and the output in standard 
 *                             form. Some useful information as the path, the hop and the score obtained are provided.
 *                              The file are located in "outputs/greedy" directory.
 *    
 *    "outputs/greedy/paramGreedy" files : for all the instances it is saved the set of parameters from which is obtained
 *                                         the best score. Those parameters are used in the metaheuristic backtracking.
 *                                         The files are located in "outputs/greedy/paramGreedy" directory.
 *    
 *    "outputs/routeHops/greedy" files : for all the instances it is saved the solution obtained if hops form to read and use it in 
 *                                       the resolution of other algortims (i.e. Local Search).
 *                                       The files are located in "outputs/hops/greedy" directory.
 *  
 * Usage:
 *    ./MainGreedy.exe [version of algorithm]
 *    - version : 
 *                1 : if with default parameters
 *                2 : if with range of parameters readed by files
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
  int errors = 0, cnt_istances =  0;
  TOP_Input in;
  string line;
  vector<chaoResults> chaoRes;

  double from_wProfit = 1.0, from_wTime = 0.7, from_maxDeviation = 1.5, from_wNonCost = 0.0; // Default Parameters
  double to_wProfit = 1.5, to_wTime = 1.5, to_maxDeviation = 1.7, to_wNonCost = 0.5;
  double up_wProfit = 1.0, up_wTime = 1.0, up_maxDeviation = 1.0, up_wNonCost = 1.0;
  
  if (argc < 1) {
    cerr << argc <<  "  ERROR: insert Greedy algorithm's version [#1 or #2]" << endl;
    return 1;
  }
  //Open and write the file of results for each instance
  fs::create_directories("solutions");
  string titleFile = "solutions/SolGreedy#";
  titleFile.push_back(*argv[1]);
  ofstream solutionsStream(titleFile + ".csv");

  if(!solutionsStream) {
    throw runtime_error("  ERROR: Unable to open Solutions file");
  }

  if(*argv[1] == '2') { // Second version without default parameters, otherwise with default parameters
    //Open the file conteining the params
    ifstream paramStream("./paramIn/paramSetGr.txt"); 
    if (!paramStream) {
      throw runtime_error("  ERROR: Unable to open Parameters file");
    }

    while(getline(paramStream, line)) {
      std::istringstream iss(line); //Split the input string
      std::vector<string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

      if(results[0] == "wProfit") {
        from_wProfit = stod(results[2]);
        to_wProfit = stod(results[3]);
        up_wProfit = stod(results[4]);
      }
      if(results[0] == "wTime") {
        from_wTime = stod(results[2]);
        to_wTime = stod(results[3]);
        up_wTime = stod(results[4]);
      }
      if(results[0] == "maxDeviation") {
        from_maxDeviation = stod(results[2]);
        to_maxDeviation = stod(results[3]);
        up_maxDeviation = stod(results[4]);
      }
      if(results[0] == "wNonCost") {
        from_wNonCost = stod(results[2]);
        to_wNonCost = stod(results[3]);
        up_wNonCost = stod(results[4]);
        break;
      }
    }
    paramStream.close();
  }

  if (up_wProfit == 0.0 || up_wTime == 0.0 || up_maxDeviation == 0.0 || up_wNonCost == 0.0) {
    throw runtime_error("  ERROR: Cannot increment by 0 the parameters: Loop");
  }

  // Print the parameters readed into the solutions file
  solutionsStream << "# Param:" << "," << "[from]" << "," << "[to]" << "," << "[plus]" << endl;
  solutionsStream << "# wProfit" << "," <<
                     from_wProfit << "," <<
                     to_wProfit << "," <<
                     up_wProfit << endl;

  solutionsStream << "# wTime" << "," << 
                     from_wTime << "," << 
                     to_wTime << "," << 
                     up_wTime << endl;

  solutionsStream << "# maxDeviation" << "," << 
                     from_maxDeviation << "," << 
                     to_maxDeviation << "," << 
                     up_maxDeviation << endl;

  solutionsStream << "# wNonProfit" << "," << 
                     from_wNonCost << "," <<
                     to_wNonCost << "," << 
                     up_wNonCost << endl;

  //Open and read the file of Chao's results
  ifstream optStream("./paramIn/chaoResults.txt"); 
  if (!optStream) {
    throw runtime_error("  ERROR: Unable to open Chao's Results file");
  }
  
  // Read all the lines into chao's file
  while(getline(optStream, line)) {
    std::istringstream iss(line); //Split the input string
    std::vector<string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
    chaoRes.push_back({.file = results[0], .chaoOptimum = stod(results[1])}); //Populate the vector of chao's results
  }
  optStream.close();
    
  for (const auto &file : fs::directory_iterator("./instances")) { //For each instance
    double best = 0;
    if (file.path().extension() != ".txt")
      continue;
    
    if(file.path().filename().replace_extension("").string()[1] != '5') { /*To select only few instances selected by map*/
      continue;
    }

    if(file.path().filename().replace_extension("").string()[3] != '4') { /*To select only few instances selected by map*/
      continue;
    }

    if(file.path().filename().replace_extension("").string()[5] != 'q') { /*To select only few instances selected by map*/
      continue;
    }


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

    TOP_Output out(in);
    
    // Solve all the instances with the set of parameters and print the outputs
    for (double wProfit = from_wProfit; wProfit <= to_wProfit; wProfit += up_wProfit) {
      for (double wTime = from_wTime; wTime <= to_wTime; wTime += up_wTime) {
        for(double maxDeviation = from_maxDeviation; maxDeviation <= to_maxDeviation; maxDeviation += up_maxDeviation) {
          for(double wNonCost = from_wNonCost; wNonCost <= to_wNonCost; wNonCost += up_wNonCost) {
            
            cerr << "LOG: Solving with parmeters <wProfit: " << wProfit <<
                    ", wProfit: " << wTime << 
                    ", maxDeviation: " << maxDeviation << 
                    ", wNonCost: " << wNonCost << ">" << endl;

            out.Clear();
            SolverAll(in, out, rng, wProfit, wTime, maxDeviation, wNonCost);

            if (out.PointProfit() > best) {
              best = out.PointProfit();
              if (!out.Feasible()) {
                cerr << "  Invalid solution" << endl;
                cout << file.path().filename() << ',' << -1 << endl;
              }
              else {
                cerr << "  Solution found: " << out.PointProfit() << endl;
                cout << file.path().filename() << ',' << out.PointProfit() << endl;
              }

              { // Print the outputs on file

                string titleDir = "outputs/greedy/outGreedy/#";
                titleDir.push_back(*argv[1]);
                fs::create_directories(titleDir);
                ofstream os(titleDir / file.path().filename().replace_extension(".out"));
                if (!os) {
                  ++errors;
                  cerr << "  ERROR: Unable to open output file" << endl;
                  continue;
                }
                os << in << out;
              }

              { // Print the Hops as outputs on file
                string titleDir = "outputs/routeHops/greedy/#";
                titleDir.push_back(*argv[1]);
                fs::create_directories(titleDir);
                ofstream os(titleDir / file.path().filename().replace_extension(".out"));
                if (!os) {
                  ++errors;
                  cerr << "  ERROR: Unable to open output file" << endl;
                  continue;
                }
                os << out;
              }
  
              { // Print the parameters of optimum on file
                string titleDir = "outputs/greedy/paramGreedy/#";
                titleDir.push_back(*argv[1]);
                fs::create_directories(titleDir);
                ofstream os(titleDir / file.path().filename().replace_extension(".out"));
                if (!os) {
                  ++errors;
                  cerr << "  ERROR: Unable to open output file" << endl;
                  continue;
                }
                os << "Profit found: " << out.PointProfit() << endl;
                os << "Param wProfit: " << wProfit << endl;
                os << "Param wTime: " << wTime << endl;
                os << "Param maxDeviation: " << maxDeviation << endl;
                os << "Param wNonCost: " << wNonCost << endl;
              } 
            }
          }
        }            
      }
    }
        
    if(out.PointProfit() == 0) { // No solution found, the problem is unfeasible
      { // Print the outputs on file
        string titleDir = "outputs/greedy/outGreedy/#";
        titleDir.push_back(*argv[1]);
        fs::create_directories(titleDir);
        ofstream os(titleDir / file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          cerr << "  ERROR: Unable to open output file" << endl;
          continue;
          }
        os << in << "h 0";
      }

      {
        string titleDir = "outputs/greedy/paramGreedy/#";
        titleDir.push_back(*argv[1]);
        fs::create_directories(titleDir);
        ofstream os(titleDir / file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          cerr << "  ERROR: Unable to open output file" << endl;
          continue;
        }
        os << "Profit found: " << out.PointProfit() << " (No solution found)" << endl;
        os << "Param wProfit: " << "null" << endl;
        os << "Param wTime: " << "null" << endl;
        os << "Param maxDeviation: " << "null" << endl;
        os << "Param wNonCost: " << "null" << endl;
      } 
      
      {
        string titleDir = "outputs/routeHops/greedy/#";
        titleDir.push_back(*argv[1]);
        fs::create_directories(titleDir);
        ofstream os(titleDir/ file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          cerr << "  ERROR: Unable to open output file" << endl;
          continue;
        }
        os << "h 0" << endl;
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
