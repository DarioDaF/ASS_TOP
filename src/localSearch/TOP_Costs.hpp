
#ifndef TOP_COSTS_HPP
#define TOP_COSTS_HPP

#include "TOP_Basics.hpp"
#include "TOP_Helpers.hpp"
#include <easylocal.hh>

using namespace EasyLocal::Core;

class TOP_CostProfit_Swap
 : public CostComponent<TOP_Input, TOP_State, int> {
  public:
    TOP_CostProfit_Swap(const TOP_Input& in, int w, bool hard)
     : CostComponent<TOP_Input, TOP_State, int>(in, w, hard, "TOP_CostProfit_Swap") {}
    int ComputeCost(const TOP_State& st) const; // @TODO: Implement in cpp
    void PrintViolations(const TOP_State& st, ostream& os = cout) const; // @TODO: Implement in cpp
};

class TOP_CostCar_Swap
 : public CostComponent<TOP_Input, TOP_State, int> {
  public:
    TOP_CostCar_Swap(const TOP_Input& in, int w, bool hard)
     : CostComponent<TOP_Input, TOP_State, int>(in, w, hard, "TOP_CostCar_Swap") {}
    int ComputeCost(const TOP_State& st) const; // @TODO: Implement in cpp
    void PrintViolations(const TOP_State& st, ostream& os = cout) const; // @TODO: Implement in cpp
};

// class TOP_CostW_Swap
//  : public CostComponent<TOP_Input, TOP_State, int> {
//   public:
//     TOP_CostW_Swap(const TOP_Input& in, int w, bool hard)
//      : CostComponent<TOP_Input, TOP_State, int>(in, w, hard, "TOP_CostW_Swap") {}
//     int ComputeCost(const TOP_State& st) const; // @TODO: Implement in cpp
//     void PrintViolations(const TOP_State& st, ostream& os = cout) const; // @TODO: Implement in cpp
// };


// Do not change below this line

class TOP_CostContainer {
  public:
    TOP_CostContainer(const TOP_Input& in)
     : _Car_Swap(in, 1, true), _Profit_Swap(in, 1, false) /*, _W_Swap(in, 1, true)*/  {}
    void AddCostComponents(TOP_StateManager& sm) {

      sm.AddCostComponent(_Car_Swap);

      sm.AddCostComponent(_Profit_Swap);

      // sm.AddCostComponent(_W_Swap);

    }

    TOP_CostCar_Swap _Car_Swap;

    TOP_CostProfit_Swap _Profit_Swap;

    // TOP_CostW_Swap _W_Swap;

};

#endif
