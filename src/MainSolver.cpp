#include "common/TOP_Data.hpp"
#include "greedy/Kevin.hpp"
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
 * MainSolver.cpp is a main that takes all the instances and solve them with a set of params. The results
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
  double from_a = 1, from_b = 1, from_c = 1, from_d = 1; // Default Parameters
  double to_a = 1, to_b = 1, to_c = 1, to_d = 1;
  double up_a = 0.1, up_b = 0.1, up_c = 0.1, up_d = 0.1;
        
  //Open and write the file of results for each instance
  fs::create_directories("solutions");
  ofstream solutionsStream("solutions/SolGreedy.csv");
  if(!solutionsStream) {
    throw runtime_error("  ERROR: Unable to open Solutions file");
  }

  //Open the file conteining the params
  ifstream paramStream("./paramIn/paramSetGr.txt"); 
  if (!paramStream) {
    throw runtime_error("  ERROR: Unable to open Parameters file");
  }

  while(getline(paramStream, line)) {
    std::istringstream iss(line); //Split the input string
    std::vector<string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

    if(results[0] == "A") {
      from_a = stod(results[2]);
      to_a = stod(results[3]);
      up_a = stod(results[4]);
    }
    if(results[0] == "B") {
      from_b = stod(results[2]);
      to_b = stod(results[3]);
      up_b = stod(results[4]);
    }
    if(results[0] == "C") {
      from_c = stod(results[2]);
      to_c = stod(results[3]);
      up_c = stod(results[4]);
    }
    if(results[0] == "D") {
      from_d = stod(results[2]);
      to_d = stod(results[3]);
      up_d = stod(results[4]);
      break;
    }
  }
  paramStream.close();

  if (up_a == 0.0 || up_b == 0.0 || up_c == 0.0 || up_d == 0.0) {
    throw runtime_error("  ERROR: Cannot increment by 0 the parameters: Loop");
  }

  // Print the parameters readed into the solutions file
  solutionsStream << "Param:" << ";" << "[from]" << ";" << "[to]" << ";" << "[plus]" << endl;
  solutionsStream << "A" << ";" << to_string(from_a) << ";" << to_string(to_a) << ";" << to_string(up_a) << endl;
  solutionsStream << "B" << ";" << to_string(from_b) << ";" << to_string(to_b) << ";" << to_string(up_b) << endl;
  solutionsStream << "C" << ";" << to_string(from_c) << ";" << to_string(to_c) << ";" << to_string(up_c) << endl;
  solutionsStream << "D" << ";" << to_string(from_d) << ";" << to_string(to_d) << ";" << to_string(up_d) << endl;

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

    // if(file.path().filename().replace_extension("").string()[1] < '5') { /*To select only few instances selected by map*/
    //   continue;
    // }

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
    for (double a = from_a; a <= to_a; a += up_a) {
      for (double b = from_b; b <= to_b; b += up_b) {
        for(double c = from_c; c <= to_c; c += up_c) {
          for(double d = from_d; d <= to_d; d += up_d) {
            
            // cerr << "LOG: Solving with parmeters <A: " << a << ", B: " << b << ", C: " << c << ", D: " << d << ">" << endl;

            out.Clear();
            SolverAll(in, out, rng, a, b, c, d);

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
                fs::create_directories("outputs/greedy/outGreedy");
                ofstream os("outputs/greedy/outGreedy" / file.path().filename().replace_extension(".out"));
                if (!os) {
                  ++errors;
                  cerr << "  ERROR: Unable to open output file" << endl;
                  continue;
                }
                os << in << out;
              }

              { // Print the Hops as outputs on file
                fs::create_directories("outputs/routeHops/greedy");
                ofstream os("outputs/routeHops/greedy" / file.path().filename().replace_extension(".out"));
                if (!os) {
                  ++errors;
                  cerr << "  ERROR: Unable to open output file" << endl;
                  continue;
                }
                os << out;
              }
  
              { // Print the parameters of optimum on file
                fs::create_directories("outputs/greedy/paramGreedy");
                ofstream os("outputs/greedy/paramGreedy" / file.path().filename().replace_extension(".out"));
                if (!os) {
                  ++errors;
                  cerr << "  ERROR: Unable to open output file" << endl;
                  continue;
                }
                os << "Profit found: " << out.PointProfit() << endl;
                os << "Param A: " << a << endl;
                os << "Param B: " << b << endl;
                os << "Param C: " << c << endl;
                os << "Param D: " << d << endl;
              } 
            }
          }
        }            
      }
    }
        
    if(out.PointProfit() == 0) { // No solution found, the problem is unfeasible
      { // Print the outputs on file
        fs::create_directories("outputs/greedy/outGreedy");
        ofstream os("outputs/greedy/outGreedy" / file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          cerr << "  ERROR: Unable to open output file" << endl;
          continue;
          }
        os << in << "h 0";
      }

      {
        fs::create_directories("outputs/greedy/paramGreedy");
        ofstream os("outputs/greedy/paramGreedy" / file.path().filename().replace_extension(".out"));
        if (!os) {
          ++errors;
          cerr << "  ERROR: Unable to open output file" << endl;
          continue;
        }
        os << "Profit found: " << out.PointProfit() << " (No solution found)" << endl;
        os << "Param A: " << "null" << endl;
        os << "Param B: " << "null" << endl;
        os << "Param C: " << "null" << endl;
        os << "Param D: " << "null" << endl;
      } 
      
      {
        fs::create_directories("outputs/routeHops/greedy");
        ofstream os("outputs/routeHops/greedy" / file.path().filename().replace_extension(".out"));
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
        solutionsStream << file.path().filename() << ";" << chaoRes[cnt_istances].chaoOptimum << ";" << best << ";" << 1.0 << endl;
        ++cnt_istances;
        continue;
      }
      solutionsStream << file.path().filename() << ";" << chaoRes[cnt_istances].chaoOptimum << ";" << best << ";0," << static_cast<int>(best / chaoRes[cnt_istances].chaoOptimum *10000) << endl;
      ++cnt_istances;
    }
    else { // New map found
      solutionsStream << file.path().filename() << ";" << best << ";" << "(new map)" << endl;
    } 
  }
  solutionsStream.close();
  cerr << "Total errors: " << ((double)errors) / 2 << endl;
  return 0;
}