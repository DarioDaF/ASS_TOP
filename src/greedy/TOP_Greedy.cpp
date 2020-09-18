#include "TOP_Greedy.hpp"
#include "../common/Utils.hpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

/**************
* Declaration *
**************/

/**
 * Return the distance between one point p and the last one inserted into a specified car
 *
 * @param in constant input
 * @param out constant output
 * @param p the point considered
 * @param car the car from which obtain the second point to evaluate the distance
 * @return distance between the two point
 */
double CalculateDistance(const TOP_Input& in, const TOP_Output& out, idx_t p, idx_t car);

/**
 * Verify if there is at least one car that can insert the point p
 *
 * @param in constant input
 * @param out constant output
 * @param p the point considered
 * @return true if feasible, false otherwise
 */
bool VerifyFeasibility(const TOP_Input& in, const TOP_Output& out, idx_t p);

/**
 * Evaluate with random algorithm if one partial solution could be inserted into its vector. It use a 
 * random number which is compared with the number of partial solutions already inserted into the vector.
 * In this way, the probability of one insertion is linearly proportional to the number of partial solutions 
 * already solved (EVALUATEPARTIAL_MINCOUNT_RNG is equal 100% of possibility of insertion, EVALUATEPARTIAL_MAXCOUNT_RNG is 0%).
 *
 * @param rng
 * @param totalQueuedAndSolvedSolutions
 * @return true if the random generation of number allow the insertion
 */
bool EvaluatePartial(std::mt19937& rng, int totalQueuedAndSolvedSolutions);

#define EVALUATEPARTIAL_MINCOUNT_RNG 100.0
#define EVALUATEPARTIAL_MAXCOUNT_RNG 600.0

/**
 * Calculate one component of the rating assigned to one point. Estimate the possible profit losses
 * if one point isn't chosen, by summing all the point profits in the ellipse, which area is drawn
 * from the remaining time of the car and the point p. The result is a value without a unit of measure.
 *
 * @param in constant input
 * @param out output that can be modified
 * @param p the point considered
 * @return point profits in the ellipse divided by all the points profit 
 */
double NonChoicheCost(const TOP_Input& in, TOP_Output& out, idx_t car, idx_t p, double sumProfit);

/**
 * Calculate the rating assigned to one point, which is made up of three components: the first is 
 * referring to the point profit on the mean of remaining points profit, the second one is referring 
 * to the remaining traveltime of the nearest car to the point and the impact it would have in entering 
 * that point and at least the cost (losses) of no choosing that point. All these components are weighted to adapt 
 * them to differents maps and their conformation.
 *
 * @param in constant input
 * @param out output that can be modified
 * @param p the point considered
 * @param wProfit weight that multiplies the first (profit) factor of the rating equation
 * @param wTime weight that multiplies the second (travel time) factor of the rating equation
 * @param wNonCost weight that multiplies the third (cost of no choice) factor of the rating equation
 * @return rating of the point p
 */
double RatingChoice(const TOP_Input& in, TOP_Output& out, idx_t p, double wProfit, double wTime, double wNonCost);

/**
 * After choosing one point to insert in the nearest car, determinate if there is one (or more) point 
 * to insert between the point and the last inserted in the car. The maximum deviation admitted in its path 
 * is passed by argument of the function. This function is called recursively to not let the car move randomly 
 * (i.e. avoid divergent and broken movements).
 *
 * @param in constant input
 * @param out output that can be modified
 * @param car the car which path is modified 
 * @param maxDeviationAdmitted max deviation admitted on the path of the car
 * @return integer value rappresentative of the number of points inserted by the recursive call
 */
int InsertPoint(const TOP_Input &in, TOP_Output& out, idx_t car, double maxDeviationAdmitted);

/**
 * Solve the problem with the greedy algorithm assigning to the point with the highest rating to its nearest 
 * car and the applying the InsertPoint function. Besides if there is some points with differents rating, the 
 * algorithm save the partial solution and also solve it. Not all the partial solution are saved: it is found 
 * empirically that most of all the instances have less than 100 partial solution. Only a few have more than 600.
 * If the partial_solution size is less than 100, all the instances are inserted. Between EVALUATEPARTIAL_MINCOUNT_RNG 
 * and EVALUATEPARTIAL_MAXCOUNT_RNG a random number generator is used to calculate the probability of insertion. 
 * Out of EVALUATEPARTIAL_MAXCOUNT_RNG all the partial solution are rejected.
 *
 * @param partial_solutions vector of partial solutions
 * @param in constant input
 * @param out constant output
 * @param rng seed generator to save the solution and its informations
 * @param solvedSolutions parametr that allow to evaluate the insertion of one partial solution
 * @param wProfit weight that multiplies the first (profit) factor of the rating equation
 * @param wTime weight that multiplies the second (travel time) factor of the rating equation
 * @param maxDeviationAdmitted max deviation admitted on the path of the car
 * @param wNonCost weight that multiplies the third (no choosing cost or losses) factor of the rating equation
 * @return [void]
 */
void PointToCarAssignment(vector<TOP_Output>& partial_solutions, const TOP_Input& in, TOP_Output& out, std::mt19937& rng, int solvedSolutions, double wProfit, double wTime, double maxDeviationAdmitted, double wNonCost);

/******************
 * Implementation *
 ******************/

double CalculateDistance(const TOP_Input& in, const TOP_Output& out, idx_t p, idx_t car) {
  return in.Distance(p, out.CarPoint(car));
}

bool VerifyFeasibility(const TOP_Input& in, const TOP_Output& out, idx_t p) {
  NumberRange<idx_t> carIdxs(in.Cars());

  // Verify that there is at least one car that can insert the point
  for(const auto& car : carIdxs) {
    if(out.SimulateMoveCar(car, p).feasible) {
      return true;
    }
  }
  return false; // Otherwise
}

bool EvaluatePartial(std::mt19937& rng, int totalQueuedAndSolvedSolutions) {
  if(totalQueuedAndSolvedSolutions <= EVALUATEPARTIAL_MINCOUNT_RNG) {
    return true;
  }
  if(totalQueuedAndSolvedSolutions >= EVALUATEPARTIAL_MAXCOUNT_RNG) {
    return false;
  }

  uniform_real_distribution<double> distribution(0.0, 1.0);
  double resultPerc = distribution(rng);
  double passPerc = ((double)totalQueuedAndSolvedSolutions - EVALUATEPARTIAL_MINCOUNT_RNG) / (EVALUATEPARTIAL_MAXCOUNT_RNG - EVALUATEPARTIAL_MINCOUNT_RNG);
  
  // cerr << "LOG: <" << resultPerc << " on " << passPerc << ">" << endl;
  return resultPerc >= passPerc;
}

double NonChoicheCost(const TOP_Input& in, TOP_Output& out, idx_t car, idx_t p, double sumProfit) {
  double profitEllipse = 0.0;

  out.MoveCar(car, p);
  for(idx_t point = 0; point < in.Points(); ++point) {
    if(out.Visited(point)) { // If is not already visited
      continue;
    }
    if(out.SimulateMoveCar(car, point).feasible) { // If can be reached by the selected car
      profitEllipse += in.Point(point).Profit();
      break;
    }
  }
  out.RollbackCar(car);
  if(sumProfit == 0.0) {
    return INFINITY;
  }
  return profitEllipse / sumProfit;
}

double RatingChoice(const TOP_Input& in, TOP_Output& out, idx_t p, double wProfit, double wTime, double wNonCost) {
  NumberRange<idx_t> carIdxs(in.Cars());
  NumberRange<idx_t> pointIdxs(in.Points());
  double profit = in.Point(p).Profit();
  double notVisitedCount = 0.0;
  double sumProfit = 0.0;

  // If the point is already visited or unfeasible, waste it by putting it in the vector's queue
  if(out.Visited(p) || !VerifyFeasibility(in, out, p)) {                                                        
    return -INFINITY;
  }
  
  // Factor dependent on the profit   
  for(const auto& p : pointIdxs) { 
    if(!out.Visited(p)) {
      sumProfit += in.Point(p).Profit();
      ++notVisitedCount;
    }
  }

  double meanProfit;
  if(notVisitedCount == 0.0) {
    meanProfit = INFINITY;
  }
  else {
    meanProfit = sumProfit / notVisitedCount;
  }

  // Factor dependent on the traveltime
  idx_t chosenCar = *min_element(carIdxs.begin(), carIdxs.end(), [&in, &out, &p](idx_t c1, idx_t c2) {
    return CalculateDistance(in, out, p, c1) < CalculateDistance(in, out, p, c2);
  });
  double gamma = out.TravelTime(chosenCar) / in.MaxTime();

  double extraTravelTimeNorm;
  if(in.MaxTime() - out.TravelTime(chosenCar) == 0.0) {
    extraTravelTimeNorm = INFINITY;
  }
  else {
    extraTravelTimeNorm = out.SimulateMoveCar(chosenCar, p).extraTravelTime / (in.MaxTime() - out.TravelTime(chosenCar));
  }
  
  // Factor dependent on the cost (losses) of chosing another point
  double noChoice = NonChoicheCost(in, out, chosenCar, p, sumProfit);

  return (profit / meanProfit) * wProfit - (gamma * extraTravelTimeNorm) * wTime + noChoice * wNonCost;
}

int InsertPoint(const TOP_Input &in, TOP_Output& out, idx_t car, double maxDeviationAdmitted) {
  NumberRange<idx_t> pointIdxs(in.Points());
  vector<idx_t> inEllipse;

  if(out.CarPoint(car) != in.StartPoint()) { // Only if the car has already move (debugging porpouse)

    // Add to the vector only the points which deviation from the path is admitted
    for(idx_t point = 1; point < (in.Points() - 1); ++point) {
      double ellipseDist = 
        in.Distance(out.CarPoint(car), point) + 
        in.Distance(out.Hop(car, out.Hops(car) - 2), point) - 
        in.Distance(out.CarPoint(car), out.Hop(car, out.Hops(car) - 2));
      
      // cerr <<
      //   "LOG: from " << out.Hop(car, out.Hops(car) - 2) << " to " <<
      //   out.CarPoint(car) << " taking " << point << ": " << ellipseDist << endl;

      if(!out.Visited(point) && ellipseDist <= maxDeviationAdmitted) {
        inEllipse.push_back(point);
        // cerr << "LOG: " << point << "and " << ellipseDist << endl;
        // cerr <<
        //   "LOG: " << in.Point(out.CarPoint(car)).Distance(in.Point(point)) << " " <<
        //   in.Point(out.Hop(car, out.Hops(car)-2)).Distance(in.Point(point)) << " " << maxDeviationAdmitted << endl;
      }
    }

    // cerr << "LOG: list " << out.Hop(car, out.Hops(car) - 2) << " -> " << out.CarPoint(car) << ": ";
    // for(idx_t p : inEllipse) { cerr << p << ", "; }
    // cerr << endl;

    if(!inEllipse.empty()) { // If there ir at least one point to insert

      // Rollback the car to insert the point 
      idx_t lastNode = out.CarPoint(car);
      out.RollbackCar(car);
      // cerr << "LOG: Rollback car: " << car << " of point: " << lastNode << endl;
  
      // Order all the points by distance from the last point inserted into the car and the select the nearest
      std::sort(inEllipse.begin(), inEllipse.end(), [ &in, &out, car ](idx_t p1, idx_t p2) {
        return in.Distance(p1, out.CarPoint(car)) < in.Distance(p2, out.CarPoint(car));
      });
      idx_t node = inEllipse.front();
      // cerr << "LOG: Point choose: " << node << " into car " << car << endl;

      double dist =
        out.TravelTime(car) -
        in.Distance(out.CarPoint(car), in.EndPoint()) +
        in.Distance(out.CarPoint(car), node) +
        in.Distance(node, lastNode) +
        in.Distance(lastNode, in.EndPoint());
      // cerr << "LOG: Distance of point: " << node << " -> " << dist << " on " << in.MaxTime() << endl;

      // Verify if it is possible to insert the point into the car path, otherwise throw an error 
      if(dist <= in.MaxTime()) {
        if(!out.MoveCar(car, node, false).feasible) {
          throw runtime_error("ERROR: Insert failed but check feasibility passed");
        }
        else {
          // cerr << "LOG: Inserted point " << node << " with cnt : " << cnt << endl;
          if(!out.MoveCar(car, lastNode, false).feasible) {
            throw runtime_error("ERROR: Cannot reinsert the last point but check feasibility passed");
          }
          return 1 + InsertPoint(in, out, car, maxDeviationAdmitted);
        }
      } 
      else { // If there isn't enough travel time left
        if(!out.MoveCar(car, lastNode, false).feasible) {
          runtime_error("ERROR: Cannot reinsert last point but check feasibility passed");
        }
        return 0;
      }
      runtime_error("ERROR: This point of the code cannot be reached : check feasibility"); // Symbolic (Debugging porpouse)
    }
    else { // If the ellipse is empty
      return 0;
    }
  }
  else { // If it hasn't already move (debugging porpouse)
    runtime_error("ERROR: The car hasn't already moved and it is impossible to execute InsertPoint");
    return 0;
  }
}

void PointToCarAssignment(vector<TOP_Output>& partial_solutions, const TOP_Input& in, TOP_Output& out, std::mt19937& rng, int solvedSolutions, double wProfit, double wTime, double maxDeviationAdmitted, double wNonCost) {
  NumberRange<idx_t> carIdxs(in.Cars());
  NumberRange<idx_t> pointIdxs(in.Points());
  vector<bool> markedCars(in.Cars());

  while(true) {
    
    // Look for the best points insertion based on the rating of the point (to nearest car)
    auto maxPoints = min_elements(in.Points(), so_negcmp<double>, [&in, &out, &wProfit, &wTime, &wNonCost] (idx_t p) -> double {
      if(!VerifyFeasibility(in, out, p)) {
        return -INFINITY;
      }
      return RatingChoice(in, out, p, wProfit, wTime, wNonCost);
    });
    // cerr << "LOG: Size " << maxPoints.size() << endl;

    // Choose a random point as main path (tie breaking)
    idx_t mainBranchPoint = std::uniform_int_distribution<idx_t>(0, maxPoints.size()-1)(rng);

    for(int i = 0; i < maxPoints.size(); ++i) {
      if(i == mainBranchPoint) {
        continue; // Main branch, no need to queue
      }

      // Assign the current point to its nearest car
      idx_t chosenPoint = maxPoints[i];
      idx_t chosenCar = *min_element(carIdxs.begin(), carIdxs.end(), [&in, &out, &markedCars, &chosenPoint](idx_t c1, idx_t c2) {
        // If marked travel time is infinite!
        if(markedCars[c1]) return false;
        if(markedCars[c2]) return true;
        return CalculateDistance(in, out, chosenPoint, c1) < CalculateDistance(in, out, chosenPoint, c2);
      });

      if(markedCars[chosenCar]) {
        continue;
      }

      // Evaluate the partial solution
      if(!out.Visited(chosenPoint) && out.MoveCar(chosenCar, chosenPoint, false).feasible) { 
        int backhops = InsertPoint(in, out, chosenCar, maxDeviationAdmitted);
        
        // cerr << "LOG: partialCounter: " << partialCounter << endl; 
        
        // Evaluate if insert the partial solution based on the partial solutions processed
        if(EvaluatePartial(rng, solvedSolutions + partial_solutions.size())) {
          partial_solutions.push_back(out);
        } else {
          // Reject or substitute
          // Probability of rejection on subsequent calls sould be uniform (even though order is dependent, and sort of depth first)
          // To solve this sould use random to select next partial solution, but this can reduce tree traversal
          // Because initial splits will be randomized
          // For semplicity an ordered evaluation is preferred and 50% rejection or random substitution is applied
          if(uniform_int_distribution<int>(0, 1)(rng) && partial_solutions.size() > 0) {
            idx_t subedPartial = uniform_int_distribution<idx_t>(0, partial_solutions.size()-1)(rng);
            partial_solutions[subedPartial] = out;
          }
        }
        
        // Rollback InsertPoint
        for(int j = 0; j <= backhops; j++){
          out.RollbackCar(chosenCar);
        }  
      }
    }

    idx_t chosenPoint = maxPoints[mainBranchPoint];
    // cerr << "LOG: Assign car " << chosenCar << " to " << chosenPoint << endl;
    
    // Assign the point to the nearest car
    idx_t chosenCar = *min_element(carIdxs.begin(), carIdxs.end(), [&in, &out, &markedCars, &chosenPoint](idx_t c1, idx_t c2) {
      // If marked travel time is infinite!
      if(markedCars[c1]) return false;
      if(markedCars[c2]) return true;
      return CalculateDistance(in, out, chosenPoint, c1) < CalculateDistance(in, out, chosenPoint, c2);
    });

    if(markedCars[chosenCar]) {
      break; // All cars are marked
    }

    if(out.Visited(chosenPoint) || !out.MoveCar(chosenCar, chosenPoint, false).feasible) { 
      // Already finished all the point for the selected car
      markedCars[chosenCar] = true;
      continue;
    }
    else {
      // cerr << "LOG: Hops before : " << out.Hops(chosenCar) << endl;
      InsertPoint(in, out, chosenCar, maxDeviationAdmitted);
      // cerr << "LOG: Hops after : " << out.Hops(chosenCar) << endl;
    }
  }
  
  // for(const auto& cr : carIdxs) { // Print the remaining travel time
  //   cerr << 
  //     "TravelTime car " << cr << " " << out.TravelTime(cr) <<
  //     " on MaxTravel " << in.MaxTime() << " (" << out.TravelTime(cr) / in.MaxTime() * 100 << "%)" << endl;
  // }

  // cerr << "LOG: counter of partial solution inserted " << partialCounter << endl;
}

void GreedySolver(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, double wProfit, double wTime, double maxDeviationAdmitted, double wNonCost) {
  vector<TOP_Output> partial_solutions;
  int solvedSolutions = 0;

  partial_solutions.clear(); // Start solving
  partial_solutions.push_back(out);

  while(!partial_solutions.empty()) { // While all the partial solution are solved
    
    auto lastSol = partial_solutions.back(); // To next partial solution
    partial_solutions.pop_back();
    
    PointToCarAssignment(partial_solutions, in, lastSol, rng, solvedSolutions, wProfit, wTime, maxDeviationAdmitted, wNonCost); // Solve
    ++solvedSolutions; 

    if(lastSol.PointProfit() > out.PointProfit()) { // Update the best solution found
      out = lastSol;
      
      // if(out.PointProfit() != 0) { // Print solution improvement
      //   cerr << 
      //     "LOG: found better solution of " <<  
      //     static_cast<double>(lastSol.PointProfit()) / static_cast<double>(out.PointProfit()) * 100 - 100 <<
      //     " % from the previous one" << endl;
      // }
    }
  }
  // cerr << "LOG: Currently solved the instance " << solvedSolutions << " times" << endl;
}

void GreedyRangeSolver(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, std::ostream& log) {
  TOP_Output curr_out(in);
  int bestProfit = 0;
  
  double wTimeStep = 0.1;
  double wNonCostStep = 0.5;

  for(double wTime = 0.1; wTime <= 3.7; wTime += wTimeStep) {
    if(wTime > 1.2 && wTime < 2.9) {
      wTime = 2.9;
      wTimeStep = 0.5;
    }

    for(double wNonCost = 0; wNonCost <= 5; wNonCost += wNonCostStep) {
      if(wNonCost > 1 && wNonCost < 5) {
        wNonCost = 5;
        wNonCostStep = 0.5;
      }

      for(double maxDeviation = 0; maxDeviation <= 6; maxDeviation += 0.01) {
        curr_out.Clear();
        GreedySolver(in, curr_out, rng, 1, wTime, maxDeviation, wNonCost);
        if(curr_out.PointProfit() > bestProfit) {
          out = curr_out;
          bestProfit = out.PointProfit();
          log << "Found better solution: " << bestProfit << " with (" << wTime << ", " << maxDeviation << ", " << wNonCost << ")" << std::endl;
        }
      }
    }
  }
}
