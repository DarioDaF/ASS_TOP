#include "TOP_Data.hpp"

#include <fstream>

using namespace std;

/***************
 * Declaration *
 ***************/

#pragma region TOP_Point

// IO

/**
 * Print all the information about one point: coordinates and profit
 *
 * @param os ostream variable 
 * @param p constant point p
 * @return the ostream variable
 */
ostream& operator<<(ostream& os, const TOP_Point& p) {
  os << p.x << "\t" << p.y << "\t" << p.p;
  return os;
}

/**
 * Read all the information about one point: coordinates and profit
 *
 * @param is variable istream
 * @param p point p
 * @return the istream variable
 */
istream& operator>>(istream& is, TOP_Point& p) {
  is >> p.x >> p.y >> p.p;
  return is;
}

#pragma endregion

#pragma region TOP_Input

/**
 * Verify if there is the input instance selected and generate the input
 *
 * @param filename the string with the file name 
 */
TOP_Input::TOP_Input(const std::string& filename) {
  ifstream is(filename);
  if(!is) {
    throw logic_error("TOP_Input::TOP_Input unable to open file");
  }
  is >> *this;
}

/**
 * Clear the input
 *
 * @param None
 * @return [void]
 */
void TOP_Input::Clear() {
  cars = 0;
  max_time = 0;
  points.clear();
}

// IO

/**
 * Read the input from the instance file
 *
 * @param is istream variable 
 * @param in input that can be modified
 * @return istream variable
 */
istream& operator>>(istream& is, TOP_Input& in) {
  in.Clear();
  string _;
  int nPoints = 0;
  is >> _ >> nPoints >> _ >> in.cars >> _ >> in.max_time;
  in.points.resize(nPoints);
  for(int i = 0; i < nPoints; ++i) {
    TOP_Point p;
    is >> p;
    in.points[i] = p;
  }
  return is;
}

/**
 * Write the input in the same way that was red from the instance file for more modularity for others algorithms
 *
 * @param os variable ostream
 * @param in constant input
 * @return ostream variable
 */
ostream& operator<<(ostream& os, const TOP_Input& in) {
  os << "n " << in.points.size() << endl;
  os << "m " << in.cars << endl;
  os << "tmax " << in.max_time << endl;
  for(int i = 0; i < in.points.size(); ++i) {
    os << in.points[i] << endl;
  }
  return os;
}

#pragma endregion

#pragma region TOP_Output

/**
 * Compare two outputs from both inputs and outputs (car hops) structures
 *
 * @param out1 first constant output 
 * @param out2 second constant output
 * @return true if they are equal, false otherwise
 */
bool operator==(const TOP_Output& out1, const TOP_Output& out2) {
  return out1.in == out2.in && out1.car_hops == out2.car_hops;
}

/**
 * Clear the outputs 
 *
 * @param None
 * @return [void]
 */
void TOP_Output::Clear() {
  fill(visited.begin(), visited.end(), 0); // Clear the visited points' vector
  visited[in.StartPoint()] += in.Cars();
  visited[in.EndPoint()] += in.Cars();

  for(idx_t car = 0; car < in.Cars(); ++car) { // Clear the car
    car_hops[car].clear();
  }

  double zeroTravelTime = in.Point(in.StartPoint()).Distance(in.Point(in.EndPoint())); // Clear the travel time
  fill(travel_time.begin(), travel_time.end(), zeroTravelTime);

  point_profit = in.Point(in.StartPoint()).Profit(); // Clear the profit
  if(in.EndPoint() != in.StartPoint()) {
    point_profit = in.Point(in.EndPoint()).Profit();
  }

  time_violations = 0; // Clear the time violations
}

/**
 * Move the specified car to the destination point if the insert it is feasible (not exceed the
 * maximum travel time available). For debugging porpouse the insert can be forced
 *
 * @param car the car to move
 * @param dest the destination point to insert
 * @param force boolean value that allow to force the insert if it's true
 * @return struct of the simulation of the move
 */
const TOP_Output::SimulateMoveCarResult TOP_Output::MoveCar(idx_t car, idx_t dest, bool force) {
  const auto res = SimulateMoveCar(car, dest);
  if(force || res.feasible) {
    IncrementTravelTime(car, res.extraTravelTime);

    car_hops[car].push_back(dest);
    IncrementVisited(dest, 1);
  }
  return res;
}

/**
 * Rollback one selected car: remove the its last point inserted 
 *
 * @param car the car to rollback
 * @return the index of the point removed
 */ 
idx_t TOP_Output::RollbackCar(idx_t car) {
  if(car_hops.empty()) {
    throw new runtime_error("Cannot rollback a car that has not moved"); // Debugging and hardening the code
  }

  idx_t last = CarPoint(car); // Update all the data about both the point and the car
  IncrementVisited(last, -1);
  car_hops[car].pop_back();
  IncrementTravelTime(car, -SimulateMoveCar(car, last).extraTravelTime);

  return last;
}

/**
 * Simulate the insert of one point into the last place of the car path and calculate the time
 * needed for the car to reach the point. In the end evaluate if the move is feasible without 
 * moving the really the car.  
 *
 * @param car the car to rollback
 * @param dest the destination point to insert
 * @return the struct with the simulation result
 */ 
const TOP_Output::SimulateMoveCarResult TOP_Output::SimulateMoveCar(idx_t car, idx_t dest) const {
  const TOP_Point pPart = in.Point(CarPoint(car));
  const TOP_Point pEnd = in.Point(in.EndPoint());
  const TOP_Point pDest = in.Point(dest);
  
  TOP_Output::SimulateMoveCarResult res;
  res.extraTravelTime = pPart.Distance(pDest) + pDest.Distance(pEnd) - pPart.Distance(pEnd); // Evaluate the additional distance 
  res.feasible = res.extraTravelTime + TravelTime(car) <= in.MaxTime(); // And the consequent feasibility
  return res;
}

/**
 * Insert the specified destination point dest into the car position specified by hop. The insertion 
 * can be done, simulated or forced (debugging porpouse) with the parameter mode. Finally evaluate the
 * maximum travel time and the consequent feasibility.
 *
 * @param car the car in wich insert the point
 * @param hop position in which insert the point
 * @param dest the destination point to insert
 * @param mode the modality in which insert the point
 * @returns res the struct of the result of the insertion
 */ 
const TOP_Output::SimulateMoveCarResult TOP_Output::InsertHop(idx_t car, idx_t hop, idx_t dest, TOP_Output::InsertMode mode) {
  TOP_Output::SimulateMoveCarResult res;
  res.extraTravelTime = extraDistance(in.Point(Hop(car, hop - 1)), in.Point(dest), in.Point(Hop(car, hop)));
  res.feasible = TravelTime(car) + res.extraTravelTime <= in.MaxTime();

  if(mode != SIMULATE && (mode == FORCE || res.feasible)) { // Update the data of both the car and the point
    IncrementTravelTime(car, res.extraTravelTime);

    car_hops[car].emplace(car_hops[car].begin() + hop - 1, dest);
    IncrementVisited(dest, 1);
  }
  return res;
}

/**
 * Remove the specified point from the car position specified by hop. 
 *
 * @param car the car in wich insert the point
 * @param hop position in which insert the point
 * @return the struct of the result of remove action
 */
const TOP_Output::SimulateMoveCarResult TOP_Output::RemoveHop(idx_t car, idx_t hop) {
  idx_t last = Hop(car, hop);
  car_hops[car].erase(car_hops[car].begin() + hop - 1); // Remove the point

  IncrementVisited(last, -1); // Update the data of both the car and the point
  double extraDist = extraDistance(in.Point(Hop(car, hop - 1)), in.Point(last), in.Point(Hop(car, hop)));
  IncrementTravelTime(car, -extraDist);

  return (TOP_Output::SimulateMoveCarResult){ .feasible = TravelTime(car) <= in.MaxTime(), .extraTravelTime = extraDist };
}

// Internals

/**
 * Calculate the distance between three points to evaluate the extra distance 
 * derivated an insertion or a move 
 *
 * @param pStart first and starting point
 * @param pNew second and middle point
 * @param pEnd third and end point
 * @return the sum of the distance between them
 */
double extraDistance(TOP_Point pStart, TOP_Point pNew, TOP_Point pEnd) {
  return pStart.Distance(pNew) + pNew.Distance(pEnd) - pStart.Distance(pEnd);
}

/**
 * Update the data of the both the increment/decrement of travel time and/or 
 * violations derivated from one insertion (or remove) or one move (or rollback) 
 *
 * @param car car to update
 * @param extraTravel the time travel to sum to the current car's travel time
 * @return [void]
 */
void TOP_Output::IncrementTravelTime(idx_t car, double extraTravel) {
  bool pre_violation = travel_time[car] > in.MaxTime(); // verify post and pre violation
  travel_time[car] += extraTravel;
  bool post_violation = travel_time[car] > in.MaxTime();

  if(pre_violation && !post_violation) { // Update violations
    --time_violations;
  }
  if(!pre_violation && post_violation) {
    ++time_violations;
  }
}

/**
 * Update the data of the both the increment/decrement of count of visits of one point
 * and the total profit derivated from one insertion (or remove) or one move (or rollback) 
 *
 * @param point point to update
 * @param count the number of time that the point has been visited in the action
 * @return [void]
 */
void TOP_Output::IncrementVisited(idx_t point, int count) {
  bool pre_visited = visited[point] > 0; // Verify pre and post visits
  visited[point] += count;
  bool post_visited = visited[point] > 0;

  if(pre_visited && !post_visited) { // Update the total profit
    point_profit -= in.Point(point).Profit();
  }
  if(!pre_visited && post_visited) {
    point_profit += in.Point(point).Profit();
  }
}

/*

// Unoptimized functions

bool TOP_Output::PartiallyFeasible() const {
  for(idx_t car = 0; car < in.Cars(); ++car) {
    if(TravelTime(car) > in.MaxTime()) {
      return false;
    }
  }
  return true;
}

double TOP_Output::TravelTime(idx_t car) const {
  if(car_hops[car].size() <= 0) return 0;
  double travel_time = 0;
  idx_t p0 = car_hops[car][0];
  for(idx_t hop = 1; hop < car_hops[car].size(); ++hop) {
    idx_t p1 = car_hops[car][hop];
    travel_time += in.Point(p0).Distance(in.Point(p1));
    p0 = p1;
  }
  return travel_time;
}

int TOP_Output::PointProfit() const {
  int profit = 0;
  for(idx_t point = 0; point < in.Points(); ++point) {
    if(visited[point]) {
      profit += in.Point(point).Profit();
    }
  }
  return profit;
}

*/

// IO

/**
 * Print the output in a standard format to better modularity with other algorithms 
 *
 * @param os ostream variable
 * @param out constant output
 * @return ostream variable
 */
ostream& operator<<(ostream& os, const TOP_Output& out) {
  int hops = 0;
  for(idx_t car = 0; car < out.in.Cars(); ++car) {
    hops += out.car_hops[car].size();
  }

  os << "h " << hops << endl;
  for(idx_t car = 0; car < out.in.Cars(); ++car) {
    for(auto hop : out.car_hops[car]) {
      os << car << "\t" << hop << endl;
    }
  }

  return os;
}

/**
 * Read the output from the standard format
 *
 * @param is istream variable
 * @param out output modified by reading
 * @return ostream variable
 */
istream& operator>>(istream& is, TOP_Output& out) {
  out.Clear();
  string _;
  int nHops = 0;
  is >> _ >> nHops;
  for(int i = 0; i < nHops; ++i) {
    int car, dest;
    is >> car >> dest;
    out.MoveCar(car, dest);
  }
  return is;
}

#pragma endregion
