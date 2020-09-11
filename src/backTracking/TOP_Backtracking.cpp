#include "TOP_Backtracking.hpp"
#include "../common/Utils.hpp"

#include <algorithm>
#include <vector>

using namespace std;

/***************
 * Declaration *
 ***************/

/**
 * Struct to better manage the choice into the branch tree: reppresent
 * the couple of point (with its rating) and car choose in the GoToChild 
 * or GoToParent moves 
 */
struct pointRating { 
  idx_t point;
  double rating;
  idx_t car;
};

/**
 * Choose the car to assign to one specific point, in particular it choose only 
 * the car feasible and the nearest to the point. 
 * 
 * @param current class that represent the current state of the problem
 * @param carSorted 
 * @param c
 * @param p 
 * @return 
 */
idx_t choseCar(TOP_Node& current, vector<idx_t> carSorted, idx_t c, idx_t p);

/**
 * Verify if there is at least one car that can insert the point p
 *
 * @param current class that represent the current state of the problem
 * @param p the point considered
 * @return true if feasible, false otherwise
 */
bool verifyFeasibility(const TOP_Node& current, idx_t p);

/**
 * Calculate one component of the rating assigned to one point. Estimate the possible profit losses
 * if one point isn't chosen, by summing all the point profits in the ellipse, which area is drawn
 * from the remaining time of the car and the point p. The result is a value without a unit of measure.
 *
 * @param current class that represent the current state of the problem 
 * @param car the car from which it could be calculate the rating 
 * @param p the point considered
 * @param sumProfit the profit sum of all points profit 
 * @return point profits in the ellipse divided by all the points profit 
 */
double NonChoicheCost(TOP_Node& current, idx_t car, idx_t p, double sumProfit);

/**
 * Calculate the rating assigned to one point, which is made up of three components: the first is 
 * referring to the point profit on the mean of remaining points profit, the second one is referring 
 * to the remaining traveltime of the nearest car to the point and the impact it would have in entering 
 * that point and at least the cost (losses) of no choosing that point. The function is provided of boolean 
 * variable that distinguish if the rating is calculated from the nearest car (false) or from all the cars 
 * that aren't the nearest (true) to the considered poit p passed by variable.
 * 
 * @param ocurrent class that represent the current state of the problem 
 * @param p the point considered
 * @param c the car that the rating is calculated
 * @param Mod bool value that distinguish the two case of Rating points
 * @return rating of the point p
 */
double RatingChoice(TOP_Node& current, idx_t p, idx_t c, bool Mod, double wProfit, double wTime, double wNonCost);

/**
 * After choosing one point to insert in the nearest car, determinate if there is one point to insert between 
 * the point and the last inserted in the car, condition of inserting: the nearest point. The maximum deviation 
 * admitted in its path is passed by argument of the function and it is input from the greedy parameters
 *
 * @param ocurrent class that represent the current state of the problem 
 * @param car the car which path is modified 
 * @param maxDeviationAdmitted max deviation admitted on the path of the car
 * @return integer value rappresentative of the index of point inserted by the function
 */
idx_t InsertPoint(TOP_Node& current, idx_t point, idx_t car, double maxDeviationAmmitted);

/**
 * Based on the current state, this function generate a vector of couple car-point ordered by rating.
 * The model of the function is similar to the greedy algorithm and use its parameters and functions 
 * appropriately modifed
 * 
 * @param current class that represent the current state of the problem 
 * @param maxDeviation max deviation admitted in the path for InsertPoint (metaheuristic mode)
 * @return the vector of the couple of points and cars ordered by their rating
 */
std::vector<pointRating> ratingVectorGenerator(TOP_Node& current, double wProfit, double wTime, double maxDeviation, double wNonCost);

/******************
 * Implementation *
 ******************/

idx_t choseCar(TOP_Node& current, vector<idx_t> carSorted, idx_t c, idx_t p) {

  sort(carSorted.begin(), carSorted.end(), [&current, &p] (idx_t c1, idx_t c2) { // Order the cars from the nearest to the farthest
    return current.in.Distance(p, current.CarPoint(c1)) < current.in.Distance(p, current.CarPoint(c2));
  });

  // cerr << "LOG: car order < ";
  // for(idx_t j = 0; j < current.in.Cars(); j++) { cerr << carSorted[j] << " >"; }
  // cerr << endl;

  for(idx_t car = 0; car < current.in.Cars(); car++) { // Take only the car feasible and the nearest
    if(carSorted[car] != c || current.SimulateMoveCar(carSorted[car], p).feasible) {
      //cerr << "LOG: return car from chosecar -> " << carSorted[car] << endl;
      return carSorted[car];
    }
  }
  //cerr << "LOG: return car from chosecar -> " << current.in.Cars() << " no other car feasible" << endl;
  return current.in.Cars(); // If all cars are market, can't proceed with the move
}

bool verifyFeasibility(const TOP_Node& current, idx_t p) {
  NumberRange<idx_t> carIdxs(current.in.Cars());

  for(const auto& car : carIdxs) { // Verify that there is at least one car that can insert the point
    if(current.SimulateMoveCar(car, p).feasible) {
      return true;
    }
  }
  return false; // Otherwise
}

double NonChoicheCost(TOP_Node& current, idx_t car, idx_t p, double sumProfit) {
  double profitElipse = 0.0;
  
  current.MoveCar(car, p);
  for(idx_t point = 0; point < current.in.Points(); ++point) { // If is not already visited
    if(current.Visited(point)) {
      continue;
    }
 
    if(current.SimulateMoveCar(car, point).feasible) { // If can be reached by the selected car
      profitElipse += current.in.Point(point).Profit();
    }
  }
  current.RollbackCar(car);
  if(sumProfit == 0.0) {
    return INFINITY;
  }
  return profitElipse / sumProfit;
}

double RatingChoice(TOP_Node& current, idx_t p, idx_t c, bool Mod, double wProfit, double wTime, double wNonCost) {
  NumberRange<idx_t> carIdxs(current.in.Cars());
  NumberRange<idx_t> pointIdxs(current.in.Points());

  double profit = current.in.Point(p).Profit();
  double notVisitedCount = 0.0;
  double sumProfit = 0.0;
  idx_t chosenCar;

  if (current.Visited(p) || !verifyFeasibility(current, p)) {
    return -INFINITY;
  }
  
  // Factor dependent on the profit   
  for(const auto& p : pointIdxs) {
    if(!current.Visited(p)) {
      sumProfit += current.in.Point(p).Profit();
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

  if (Mod && c == current.in.Cars()) {
    throw runtime_error("    ERROR: This condition could not exist");
  }
  
  if (Mod) { // RatingPointModified
    chosenCar = c;
  }
  else { // Normal Rating point
    chosenCar = *min_element(carIdxs.begin(), carIdxs.end(), [&current, &p](idx_t c1, idx_t c2) {
      // If marked travel time is infinite!
      return current.in.Distance(p, current.CarPoint(c1)) < current.in.Distance(p, current.CarPoint(c2));
    });
  }
  double gamma = current.TravelTime(chosenCar) / current.in.MaxTime();

  // Factor dependent on the traveltime
  double extraTravelTimeNorm;
  if(current.in.MaxTime() - current.TravelTime(chosenCar) == 0.0) {
    extraTravelTimeNorm = INFINITY;
  }
  else {
    extraTravelTimeNorm = current.SimulateMoveCar(chosenCar, p).extraTravelTime / (current.in.MaxTime() - current.TravelTime(chosenCar));
  }

  // Factor dependent on the cost (losses) of chosing another point
  double noChoice = NonChoicheCost(current, chosenCar, p, sumProfit);

  // cerr <<
  //  "LOG: return ratingModified " << (profit/meanProfit) - (gamma*extraTravelTimeNorm) + noChoice <<
  //   " for point " << p << " and car " << chosencar << endl;

  return wProfit * (profit / meanProfit) - wTime * (gamma * extraTravelTimeNorm) + wNonCost * noChoice;
}

idx_t InsertPoint(TOP_Node& current, idx_t point, idx_t car, double maxDeviationAmmitted) {
  NumberRange<idx_t> pointIdxs(current.in.Points());
  vector<idx_t> inEllipse;

  // Add to the list all the point which distance is lower than the max deviation admitted
  for(idx_t p = 1; p < (current.in.Points() - 1); ++p) {
    // cerr << "LOG: insertPoint starting find inEllipse points " << car << " " << current.CarPoint(car) << " " << i << endl;
    if(current.Visited(p) || p == point) {
      continue;
    }
    double ellipseDist = 
      current.in.Distance(current.CarPoint(car), p) +
      current.in.Distance(p, point) - 
      current.in.Distance(current.CarPoint(car), p) ;
    
    // cerr <<
    //   "LOG: from " << current.CarPoint(car) << " to " <<  point << " taking " <<
    //   i << ": " << ellipseDist << " (" << current.Hop(car, current.Hops(car)-1) << ")" << endl;

    // cerr <<
    //   "LOG: distance 1: " << current.in.Distance(current.CarPoint(car), i) << " distance 2: " <<
    //   current.in.Distance(i, point) << " maxDeviation: " << maxDeviationAmmitted << endl;
    
    if(ellipseDist > maxDeviationAmmitted) {
      // cerr <<
      //   "LOG: Distance point " << i << " from point " << point << " is more than " <<
      //   maxDeviationAmmitted << " (" << ellipseDist << ")" << endl;
      continue;
    }
    inEllipse.push_back(p);
  }

  // cerr << "LIST: of car: " << car << " " << current.CarPoint(car) << " -> " << point << ": ";
  // for(idx_t p : inEllipse) { cerr << p << ", "; }
  // cerr << endl;

  if(inEllipse.size() > 0) { // If there is at least one point 
    // Order by decreasing distance from the starting point
    std::sort(inEllipse.begin(), inEllipse.end(), [&current, car ](idx_t p1, idx_t p2) {
      return current.in.Distance(p1, current.CarPoint(car)) < current.in.Distance(p2, current.CarPoint(car));
    });

    // Assign to the point in the first position of the list 
    idx_t node = inEllipse[0];
    //cerr << "LOG: Point choose: " << node << " into car " << car << endl;

    // Verify the feasibility of the insertion
    double dist =
      current.TravelTime(car) -
      current.in.Distance(current.CarPoint(car), node) +
      current.in.Distance(node, point) + 
      current.in.Distance(point, current.in.Points()-1);

    // cerr << "LOG: Distance of point: " << node << " -> " << dist << " on " << current.in.MaxTime() << endl;

    if(dist <= current.in.MaxTime()) { // If can be inserted into the path
      return node;
    } 
    // If can't be possible to insert, return the point passed by variable
    return point;
  }
  else { // If the ellipse is empty, return the point passed by variable
    return point;
  }
}

std::vector<pointRating> ratingVectorGenerator(TOP_Node& current, double wProfit, double wTime, double maxDeviation, double wNonCost) {
  NumberRange<idx_t> carIdxs(current.in.Cars()); 
  NumberRange<idx_t> pointIdxs(current.in.Points() - 1);
  std::vector<idx_t> carSorted = carIdxs.Vector();
  std::vector<pointRating> ratingPoints;

  // Determinate the rating for each point for the nearest car, as the greedy algorithm
  for(idx_t p : pointIdxs) {
    
    // cerr << "LOG: select point " << p << endl;
    pointRating currentPoint, otherCarPoint;
    bool alreadyInsert = false;
    double rating = RatingChoice(current, p, current.in.Cars(), false, wProfit, wTime, wNonCost);

    if(rating == -INFINITY) { // The point is already visited or unfeasible
      continue;
    }

    // Chose the couple point-car and its rating based on the couple itself
    currentPoint.car = choseCar(current, carSorted, current.in.Cars(), p);
    currentPoint.point = InsertPoint(current, p, currentPoint.car, maxDeviation); // VEDI IL C!!
    currentPoint.rating = rating;

    for(idx_t idx = 0; idx < ratingPoints.size(); idx++) { // Do not want duplicates
      if(currentPoint.point == ratingPoints[idx].point) /*&& currentPoint.car == ratingPoints[idx].car)*/ {
        alreadyInsert = true;
        break;
      }
    }
    if(alreadyInsert) {
      continue;
    }
    ratingPoints.push_back(currentPoint);

    // Determinate the rating for each point for the other cars
    for(idx_t c : carIdxs) {
      if(c == currentPoint.car) { // Avoid to use the same car as before
        continue;
      }
      // cerr << "LOG: select car " << c << endl;
      otherCarPoint.car = c;
      otherCarPoint.point = InsertPoint(current, p, c, maxDeviation); // VEDI IL C!!
      otherCarPoint.rating = RatingChoice(current, p, c, true, wProfit, wTime, wNonCost);

      alreadyInsert = false;
      for(idx_t idx = 0; idx < ratingPoints.size(); idx++) { // Do not want duplicates
        if(otherCarPoint.point == ratingPoints[idx].point && otherCarPoint.car == ratingPoints[idx].car) {
          alreadyInsert = true;
          break;
        }
      }
      if(alreadyInsert) {
        continue;
      }
      // cerr << "LOG: insert point modified " << p << " in car " << c << endl;
      ratingPoints.push_back(otherCarPoint);
    }
  }

  std::sort(ratingPoints.begin(), ratingPoints.end(), [] (const pointRating p1, const pointRating p2) {
    return p1.rating > p2.rating; // Sort by rating from higher to lower
  });

  // cerr << "LOG: vector after (";
  // for(int p = 0; p < ratingPoints.size(); p++) {
  //   cerr << ratingPoints[p].point << " " << ratingPoints[p].rating << " " << ratingPoints[p].car << ", "; }
  // cerr << ")" << endl;
  return ratingPoints; 
}

bool TOP_Walker::GoToChild(double wProfit, double wTime, double maxDeviation, double wNonCost) {
  NumberRange<idx_t> carIdxs(current.in.Cars()); 
  NumberRange<idx_t> pointIdxs(current.in.Points() - 1);
  
  std::vector<idx_t> carSorted = carIdxs.Vector();
  std::vector<pointRating> ratingPoints = ratingVectorGenerator(current, wProfit, wTime, maxDeviation, wNonCost);

  if(ratingPoints.empty()) { // If empty, can't go down to the branch
    //cerr << "LOG: empty (end of branch)" << endl;
    return false;
  }

  //Chose the next point with it's own car based on the previous choice
  for(idx_t idx = 0; idx < ratingPoints.size(); ++idx) {
    // if(idx != current.CarPoint(carAssignmentOrder.back())) {
    //   continue;
    // }
    if(current.Visited(ratingPoints[idx].point)) {
      continue;
    }

    // cerr << "LOG: " << ratingPoints[idx].car << " " << ratingPoints[idx].point << " " << ratingPoints[idx].rating << endl;
    if(current.MoveCar(ratingPoints[idx].car, ratingPoints[idx].point, false).feasible) { // Assign to its car
      carAssignmentOrder.push_back(ratingPoints[idx].car);
      // cerr << "LOG: insert point " << ratingPoints[idx].point << " into car " << ratingPoints[idx].car << endl;
      // cerr << "LOG: profit " << current.PointProfit() << endl;
      return true;
    }
  }
  //cerr << "LOG: profit " << current.PointProfit() << endl;
  return false; // Not possible to insert any point (finish that branch)
}

bool TOP_Walker::GoToSibiling(double wProfit, double wTime, double maxDeviation, double wNonCost) {
  NumberRange<idx_t> carIdxs(current.in.Cars()); 
  NumberRange<idx_t> pointIdxs(current.in.Points()-1);
  std::vector<idx_t> carSorted = carIdxs.Vector();
  
  // Get current car assignment and try to change permutation lexicographically
  if(carAssignmentOrder.empty()) { // If empty, can't go to the sibiling of the same branch
    return false;
  }

  idx_t car = carAssignmentOrder.back(); // Remove and save info about the point 
  idx_t point = current.RollbackCar(car);
  // std::cerr << "LOG: remove from " << car <<  " -> " << point << endl;

  std::vector<pointRating> ratingPoints = ratingVectorGenerator(current, wProfit, wTime, maxDeviation, wNonCost);

  // cerr << "LOG: vector Sibiling (";
  // for(int p = 0; p < ratingPoints.size(); ++p) {
  //   cerr << ratingPoints[p].point << " " << ratingPoints[p].rating << " " << ratingPoints[p].car << ", "; }
  // cerr << ")" << endl;

  for(idx_t idx = 0; idx < ratingPoints.size(); idx++) {
    if(ratingPoints[idx].point != point || (idx+1) >= ratingPoints.size()) { // At the end of the vector
      continue;
      // forse andava break e sopra anche car  NOTAMIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
    }
    if(current.Visited(ratingPoints[idx].point)) { // Skip point in vector but already visited from other cars
      continue;
    }
    if(ratingPoints[idx].point == point && ratingPoints[idx].car != car) { 
      continue; // non è lo stesso della condizione precedente?
    }
    
    // if(current.Visited(ratingPoints[idx+1].point)) {
    //   continue;
    // }
    
    if(current.MoveCar(ratingPoints[idx+1].car, ratingPoints[idx+1].point, false).feasible) { // If it is feasible to move
      carAssignmentOrder.back() = ratingPoints[idx+1].car;
      // std::cerr << "LOG: Assign to car " << ratingPoints[idx+1].car << " point " << ratingPoints[idx+1].point << std::endl;
      // std::cerr <<LOG: car << ":" << current.Hop(car, current.Hops(car)-1) << " " << ratingPoints[idx+1].point << std::endl;
      return true;
    }
  }

  current.MoveCar(car, 0, true); // Finished the sibilings: rollback to start the car
  return false;
}

bool TOP_Walker::GoToParent() {
  if(carAssignmentOrder.empty()) { // No one car has already move
    return false;
  }

  //std::cerr << "LOG: Current tree's depth " << carAssignmentOrder.size() << std::endl; 
  idx_t car = carAssignmentOrder.back();
  //std::cerr << "LOG: Rollback point " << current.CarPoint(car) << " from car " << car << endl;
  
  current.RollbackCar(car); // Go up one level into the branch
  carAssignmentOrder.pop_back(); 
  return true;
}

cost_t TOP_Node::GetMinCost() const {
  cost_t profit = PointProfit();
  for(idx_t point = 0; point < in.Points(); ++point) {
    if(Visited(point)) {
      continue;
    }
    // If can be reached by any car
    for(idx_t car = 0; car < in.Cars(); ++car) {
      if(SimulateMoveCar(car, point).feasible) {
        profit += in.Point(point).Profit();
        break;
      }
    }
  }
  return -profit;
}

NodeBound TOP_Checker::CheckNode(const TOP_Node& n) {
  // Check bounds and feasibility, can be used to update bounds
  if(!n.IsFeasible()) {
    //std::cerr << "Unfeasible" << std::endl;
    return NodeBound::NB_UnFeasible;
  }
  // Check highest score and return NonImproving...
  if(n.GetMinCost() >= bestCost) {
    //std::cerr << "NonImp " << n.GetMinCost() << std::endl;
    return NodeBound::NB_NonImproving;
  }
  //std::cerr << "Normal" << std::endl;
  return NodeBound::NB_Normal;
}
