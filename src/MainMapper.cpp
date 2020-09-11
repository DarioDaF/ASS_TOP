#include "common/TOP_Data.hpp"
#include "greedy/Kevin.hpp"
#include "common/Utils.hpp"

#include <sstream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <random>
#include <chrono>
#include <vector>
#include <string>

using namespace std;
namespace fs = std::filesystem;

/**
 * Struct that represent one map
 */
struct typeMap {
    string name;
    double meanDist, c, opt;
    double area, areaCompared;
    double maxX, maxY, minX, minY;
};

/**
 * Struct that represent Chao's results
 */
struct chaoResults {
    string file;
    double chaoOptimum;
};

/**
 * MainSolver.cpp is a main that takes all the instances and aim to found one value of paramter "C", the max deviation
 * admitted, scaled for all the maps. But our empirical research found out that there are no evidence of possibilty
 * to scale the parameter.
 * 
 * Input file:  
 *   chaoResultsModified.txt : file in which are contained Chao's results modified, in particular those instances that aren't
 *                             unfeasible or that the greedy algorithm resolve as optimal solution.
 *                             The file is located in "parametes_in" directory.
 * 
 *   "paramGreedy" files : files that contain the best parameters of greedy results.
 *                         The files are located in "outputs/greedy/paramGreedy" directory. 
 * 
 * Output files:
 *    mapScaler.txt : files that contain informations about maps obtained from the algorithm.
 *                    The file is located in "outputs/greedy/mapScaler" directory.
 *    
 * @param argc number of items in the command line
 * @param argv items in the command line
 * @return informations about all maps
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
  vector<typeMap> maps;
  vector<chaoResults> chao;
  int errors = 0, cnt_istances = 0;
  double c = 0.0, optimum = 0.0;
  TOP_Input in;
  string line;

    
  //Open and read the file of Chao's results
  ifstream optStream("./paramIn/chaoResultsModified.txt"); 
  if (!optStream) {
    std::cerr << "  ERROR: Unable to open Chao's results file" << endl;
    return 1;
  }

  // Read all the lines into chao's file
  while(getline(optStream, line)) {
    std::istringstream iss(line); //Split the input string
    std::vector<string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
      
    if(stoi(results[1]) == 0) { // Remove the input file with 0 of profit
      continue;
    }

    chao.push_back({.file = results[0], .chaoOptimum = stod(results[1])}); //Populate the vector of chao's results
    ++cnt_istances;
  }
  optStream.close();

  //Open and write the file of results for each instance
  fs::create_directories("outputs/greedy/mapScaler");
  ofstream osMap("outputs/greedy/mapScaler/mapper.txt");
  if(!osMap) {
    throw runtime_error("  ERROR: Unable to open Output file");
  }

  for(idx_t idx = 0; idx < chao.size(); ++idx) { // For every file in chao vector
    cout << "Processing: " << chao[idx].file << endl;
    {
      ifstream is("./instances/" + chao[idx].file);
      if (!is) {
        ++errors;
        throw runtime_error("  ERROR: Unable to open Instances file");
      }
      is >> in;
    }

    {
      string file = chao[idx].file.erase(chao[idx].file.size()-4);
      ifstream isPar("./outputs/greedy/paramGreedy/" + file + ".out");
      if (!isPar) {
        ++errors;
        throw runtime_error("  ERROR: Unable to open Parameter file");
      }

      // Read all the lines into chao's file
      while(getline(isPar, line)) {
        std::istringstream iss(line); //Split the input string
        std::vector<string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
        if(results[0] == "Profit") {
          optimum = stod(results[2]) / chao[idx].chaoOptimum * 100;
        }
        if(results[1] != "maxDeviation:") {
          continue;
        }
        c = stod(results[2]);
      }
    }
 
    TOP_Output out(in); 
    out.Clear();

    if (in.Points() == 0) {
      throw runtime_error("  ERROR: Empty map");
    }

    double maxX = in.Point(0).X();
    double maxY = in.Point(0).Y();
    double minX = maxX;
    double minY = maxY;

    double dist = 0, meanDist, area; 

    for(idx_t idx_p1 = 1; idx_p1 < in.Points(); idx_p1++) { // Search for informations about maps
      if(in.Point(idx_p1).X() > maxX) {
        maxX = in.Point(idx_p1).X();
      }
      if(in.Point(idx_p1).Y() > maxY) {
        maxY = in.Point(idx_p1).Y();
      }
      if(in.Point(idx_p1).X() < minX) {
        minX = in.Point(idx_p1).X();
      }
      if(in.Point(idx_p1).Y() < minX) {
        minY = in.Point(idx_p1).Y();
      }
      for(idx_t idx_p2 = 0; idx_p2 < in.Points(); idx_p2++) {
        dist += in.Distance(idx_p1, idx_p2);
      }
    }
    meanDist = dist / (in.Points() * in.Points());
    area = (maxX - minX) * (maxY - minY);
    maps.push_back({.name = chao[idx].file, .meanDist = meanDist, .c = c, .opt = optimum, .area = area,
                    .areaCompared = 0, .maxX = maxX, .maxY = maxY, .minX = minX, .minY = minY});
  }
  
  std::sort(maps.begin(), maps.end(), [] (const typeMap m1, const typeMap m2) { 
    return m1.area > m2.area;
  });

  for(idx_t idx = 0; idx < maps.size(); ++idx) {
    maps[idx].areaCompared = maps[idx].area / maps[0].area * 100;
  }

  std::sort(maps.begin(), maps.end(), [] (const typeMap m1, const typeMap m2) { 
    return m1.c > m2.c;
  });

  osMap << 
    "name | meanDist | maxDeviation | comparedDist (maxDeviation/meanDist) | opt (res/chaoOpt) | area | comparedArea (area/maxArea)"
    << endl;
  for(idx_t idx = 0; idx < maps.size(); ++idx) {
    osMap << maps[idx].name << " | " << maps[idx].meanDist <<
    " | " << maps[idx].c << " | " << maps[idx].c / maps[idx].meanDist << "% | " << maps[idx].opt << "% | " <<
    maps[idx].area << " | " << maps[idx].areaCompared << endl;
  }
  osMap.close();
}