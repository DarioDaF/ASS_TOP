#include "TOP_Data.hpp"

using namespace std;

#pragma region TOP_Point

// IO

ostream& operator<<(ostream& os, const TOP_Point& p) {
  os << p.x << "\t" << p.y << "\t" << p.p;
  return os;
}

istream& operator>>(istream& is, TOP_Point& p) {
  is >> p.x >> p.y >> p.p;
  return is;
}

#pragma endregion

#pragma region TOP_Input

void TOP_Input::Clear() {
  cars = 0;
  max_time = 0;
  points.clear();
}

// IO

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

void TOP_Output::Clear() {
  fill(visited.begin(), visited.end(), 0);
  visited[in.StartPoint()] += in.Cars();
  visited[in.EndPoint()] += in.Cars();

  for(idx_t car = 0; car < in.Cars(); ++car) {
    car_hops[car].clear();
  }

  double zeroTravelTime = in.Point(in.StartPoint()).Distance(in.Point(in.EndPoint()));
  fill(travel_time.begin(), travel_time.end(), zeroTravelTime);

  point_profit = in.Point(in.StartPoint()).Profit();
  if(in.EndPoint() != in.StartPoint()) {
    point_profit = in.Point(in.EndPoint()).Profit();
  }
  time_violations = 0;
}

const TOP_Output::SimulateMoveCarResult TOP_Output::MoveCar(idx_t car, idx_t dest, bool force) {
  const auto res = SimulateMoveCar(car, dest);
  if(force || res.feasible) {
    IncrementTravelTime(car, res.extraTravelTime);

    car_hops[car].push_back(dest);
    IncrementVisited(dest, 1);
  }
  return res;
}

idx_t TOP_Output::RollbackCar(idx_t car) {
  if(car_hops.empty()) {
    throw new runtime_error("Cannot rollback a car that has not moved");
  }

  idx_t last = CarPoint(car);

  IncrementVisited(last, -1);
  car_hops[car].pop_back();

  IncrementTravelTime(car, -SimulateMoveCar(car, last).extraTravelTime);

  return last;
}

const TOP_Output::SimulateMoveCarResult TOP_Output::SimulateMoveCar(idx_t car, idx_t dest) const {
  const TOP_Point pPart = in.Point(CarPoint(car));
  const TOP_Point pEnd = in.Point(in.EndPoint());
  const TOP_Point pDest = in.Point(dest);
  
  TOP_Output::SimulateMoveCarResult res;
  res.extraTravelTime = pPart.Distance(pDest) + pDest.Distance(pEnd) - pPart.Distance(pEnd);
  res.feasible = res.extraTravelTime + TravelTime(car) <= in.MaxTime();
  return res;
}

const TOP_Output::SimulateMoveCarResult TOP_Output::InsertHop(idx_t car, idx_t hop, idx_t dest, TOP_Output::InsertMode mode) {
  TOP_Output::SimulateMoveCarResult res;
  res.extraTravelTime = extraDistance(in.Point(Hop(car, hop - 1)), in.Point(dest), in.Point(Hop(car, hop)));
  res.feasible = TravelTime(car) + res.extraTravelTime <= in.MaxTime();

  if(mode != SIMULATE && (mode == FORCE || res.feasible)) {
    IncrementTravelTime(car, res.extraTravelTime);

    car_hops[car].emplace(car_hops[car].begin() + hop - 1, dest);
    IncrementVisited(dest, 1);
  }
  return res;
}
const TOP_Output::SimulateMoveCarResult TOP_Output::RemoveHop(idx_t car, idx_t hop) {
  idx_t last = Hop(car, hop);

  IncrementVisited(last, -1);
  car_hops[car].erase(car_hops[car].begin() + hop - 1);

  double extraDist = extraDistance(in.Point(Hop(car, hop)), in.Point(last), in.Point(Hop(car, hop + 1)));
  IncrementTravelTime(car, -extraDist);

  return (TOP_Output::SimulateMoveCarResult){ .feasible = TravelTime(car) <= in.MaxTime(), .extraTravelTime = extraDist };
}

// Internals

double extraDistance(TOP_Point pStart, TOP_Point pNew, TOP_Point pEnd) {
  return pStart.Distance(pNew) + pNew.Distance(pEnd) - pStart.Distance(pEnd);
}

void TOP_Output::IncrementTravelTime(idx_t car, double count) {
  bool pre_violation = travel_time[car] > in.MaxTime();
  travel_time[car] += count;
  bool post_violation = travel_time[car] > in.MaxTime();

  if(pre_violation && !post_violation) {
    --time_violations;
  }
  if(!pre_violation && post_violation) {
    ++time_violations;
  }
}

void TOP_Output::IncrementVisited(idx_t point, int count) {
  bool pre_visited = visited[point] > 0;
  visited[point] += count;
  bool post_visited = visited[point] > 0;

  if(pre_visited && !post_visited) {
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
