#include "TOP_Costs.hpp"
#include "../common/TOP_Data.hpp"

#include <exception>

int TOP_CostProfit_Swap::ComputeCost(const TOP_State& st) const {
  return -st.PointProfit();
}

void TOP_CostProfit_Swap::PrintViolations(const TOP_State& st, ostream& os) const {
  throw std::logic_error("TOP_CostTest::PrintViolations not implemented");
}

int TOP_CostCar_Swap::ComputeCost(const TOP_State& st) const {
  int car_violations = 0;

  for(idx_t car = 0; car < st.in.Cars(); ++car) { // Violation in the car travel time
    if(st.TravelTime(car) > st.in.MaxTime()) {
      ++car_violations;
    }
  }
  return car_violations;
}

void TOP_CostCar_Swap::PrintViolations(const TOP_State& st, ostream& os) const {
  for(idx_t car = 0; car < st.in.Cars(); ++car) { // Violation in the car travel time
    if(st.TravelTime(car) > st.in.MaxTime()) {
      cout << "Car " << car << " has traveled for " << st.TravelTime(car) - st.in.MaxTime() << " out of the MaxTime" << endl;
    }
  }
}

// int TOP_CostW_Swap::ComputeCost(const TOP_State& st) const {
//   return 0;
// }

// void TOP_CostW_Swap::PrintViolations(const TOP_State& st, ostream& os) const {
//   throw std::logic_error("TOP_CostTest::PrintViolations not implemented");
// }
