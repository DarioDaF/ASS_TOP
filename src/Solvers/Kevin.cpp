#include "Kevin.hpp"

#include "../Utils.hpp"

#include <vector>
#include <algorithm>

using namespace std;

int NonChoicheCost(const TOP_Input& in, const TOP_Output& out, idx_t car) {
  NumberRange<idx_t> pointIdxs(in.Points());
  int sumProfit = 0, profitElipse = 0;

  for(const auto& p : pointIdxs) {
    if(!out.Visited(p)) {
      sumProfit += in.Point(p).Profit();
    }
  }
  
  for(idx_t point = 0; point < in.Points(); ++point) {
    if(out.Visited(point)) {
      continue;
    }
    // If can be reached by any car
    if(out.SimulateMoveCar(car, point).feasible) {
      profitElipse += in.Point(point).Profit();
      break;
    }
  }
  return profitElipse / sumProfit;
}

double RatingChoice(const TOP_Input& in, const TOP_Output& out, idx_t car, idx_t p, double meanProfit) {
  if(out.Visited(p)) {
    return -INFINITY;
  }

  double profit = in.Point(p).Profit();
  //double alpha = profit / meanProfit;

  double gamma = out.TravelTime(car) / in.MaxTime();
  double extraTravelTimeNorm = out.SimulateMoveCar(car, p).extraTravelTime / (in.MaxTime() - out.TravelTime(car));
  double noChoice = NonChoicheCost(in, out, car);
  //return alpha * profit - gamma * extraTravelTime;
  return profit / meanProfit - gamma * extraTravelTimeNorm; + noChoice;
}

void InsertPoint(const TOP_Input &in, TOP_Output& out, idx_t car, double maxDeviationAmmitted) {
  NumberRange<idx_t> pointIdxs(in.Points());
  vector<idx_t> inEllipse;
  double c = maxDeviationAmmitted;

  // Se la macchina si è mossa
  if(out.CarPoint(car) != in.StartPoint()) {
    // Aggiungi a inEllipse i punti non visitati dentro l'ellisse di "corda" c
    for(idx_t i = 1; i < in.Points() - 1; ++i) {
      double ellipseDist = in.Distance(out.CarPoint(car), i) + in.Distance(out.Hop(car, out.Hops(car) - 2), i) - in.Distance(out.CarPoint(car), out.Hop(car, out.Hops(car) - 2));
      //cerr << "From " << out.Hop(car, out.Hops(car) - 2) << " to " << out.CarPoint(car) << " taking " << i << ": " << ellipseDist << endl;
      if(!out.Visited(i) && ellipseDist <= c) {
        inEllipse.push_back(i);
        //cerr << i << "and " << ellipseDist << endl;
        // cerr << in.Point(out.CarPoint(car)).Distance(in.Point(i)) << " " << in.Point(out.Hop(car, out.Hops(car)-2)).Distance(in.Point(i)) << " " << c << endl;
      }
    }

    // cerr << "rollback" << endl;
    idx_t lastNode = out.CarPoint(car);
    out.RollbackCar(car);

    // Ordina crescente per più vicino al punto di partenza
    std::sort(inEllipse.begin(), inEllipse.end(), [ &in, &out, car ](idx_t p1, idx_t p2) {
      return in.Distance(p1, out.CarPoint(car)) < in.Distance(p2, out.CarPoint(car));
    });

    /*
    cerr << "LIST: " << out.Hop(car, out.Hops(car) - 2) << " -> " << out.CarPoint(car) << ": ";
    for(idx_t p : inEllipse) {
      cerr << p << ", ";
    }
    cerr << endl;
    */

    for(idx_t node : inEllipse) {
      // Controlla che possa essere inserito il punto
      // TravelTime: ... -> carPoint -> end
      // ... -> carPoint -> node -> lastNode -> end
      // Aggiungi (carPoint, node) + (node, lastNode) + (lastNode, end) - (carPoint, end)
      double dist =
        out.TravelTime(car) -
        in.Distance(out.CarPoint(car), in.EndPoint()) +
        in.Distance(out.CarPoint(car), node) +
        in.Distance(node, lastNode) +
        in.Distance(lastNode, in.EndPoint());
      if(dist <= in.MaxTime()) {
        if(!out.MoveCar(car, node, false).feasible) {
          cerr << "ERROR: Insert failed but check passed?" << endl;
        }
      }
    }
    if(!out.MoveCar(car, lastNode, false).feasible) {
      cerr << "ERROR: Cannot reinsert last point" << endl;
    }
  }
}

void SolveKevin(const TOP_Input& in, TOP_Output& out, std::mt19937& rng, const json& options) {
  // Sover
  NumberRange<idx_t> carIdxs(in.Cars());
  NumberRange<idx_t> pointIdxs(in.Points());

  vector<bool> markedCars(in.Cars());

  double maxDeviationAmmitted = 2; //in.MaxTime() / in.Points();
  int notVisitedCount = 0;
  int sumProfit = 0;

  for(const auto& p : pointIdxs) {
    if(!out.Visited(p)) {
      sumProfit += in.Point(p).Profit();
      ++notVisitedCount;
    }
  }

  while(true) {
    // Assign minimum travel time car
    idx_t chosenCar = *min_element(carIdxs.begin(), carIdxs.end(), [&in, &out, &markedCars](idx_t c1, idx_t c2) {
      // If marked travel time is infinite!
      if(markedCars[c1]) return false;
      if(markedCars[c2]) return true;
      return out.TravelTime(c1) < out.TravelTime(c2);
    });

    if(markedCars[chosenCar]) {
      break; // All cars are marked
    }

    auto maxPoints = min_elements(in.Points(), so_negcmp<double>, [&in, &out, &chosenCar, &sumProfit, &notVisitedCount](idx_t p) -> double {
      if(!out.SimulateMoveCar(chosenCar, p).feasible) return -INFINITY;
      return RatingChoice(in, out, chosenCar, p, ((double)sumProfit) / notVisitedCount);
    });

    idx_t chosenPoint = maxPoints[0];
    // cerr << "  Assign car " << chosenCar << " to " << chosenPoint << endl;


    if(out.Visited(chosenPoint) || !out.MoveCar(chosenCar, chosenPoint, false).feasible) { 
      // Ho finito tutti i punti sicuramente di questa macchina
      markedCars[chosenCar] = true;
      continue;
    }
    else {
      // cerr << "pr" << endl;
      InsertPoint(in, out, chosenCar, maxDeviationAmmitted);
      // cerr << "dp" << endl;
      //maxDeviationAmmitted -= maxDeviationAmmitted * ((- out.TravelTime(chosenCar) + in.MaxTime()) / in.MaxTime()) ;
    }

    // update meanProfit
    --notVisitedCount;
    sumProfit -= in.Point(chosenPoint).Profit();
  }
}
