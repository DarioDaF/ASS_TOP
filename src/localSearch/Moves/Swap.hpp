#ifndef TOP_MOVE_SWAP_HPP
#define TOP_MOVE_SWAP_HPP

#include "../../common/TOP_Data.hpp"
#include "../TOP_Basics.hpp"
#include "../TOP_Costs.hpp"
#include <easylocal.hh>

using namespace EasyLocal::Core;

// Move

struct carHop_s {
  idx_t car;
  idx_t hop;
  bool IsValid() const {
    return this->car >= 0 && this->hop >= 1;
  }
  friend bool operator==(const carHop_s& ch1, const carHop_s& ch2) {
    return ch1.car == ch2.car && ch1.hop == ch2.hop;
  }
  friend bool operator<(const carHop_s& ch1, const carHop_s& ch2) {
    return ch1.car < ch2.car ||
      (ch1.car == ch2.car && ch1.hop < ch2.hop);
  }
  friend std::ostream& operator<<(ostream& os, const carHop_s& ch) {
    os << "(" << ch.car << ", " << ch.hop << ")";
    return os;
  }
  bool Increment(const TOP_State& st, bool countEnd = false) {
    ++hop;
    if(hop >= st.Hops(car) + (countEnd ? 1 : 0)) {
      hop = 1;
      do {
        ++car;
      } while(car < st.in.Cars() && st.Hops(car) + (countEnd ? 1 : 0) <= 1);
      if(car >= st.in.Cars()) {
        return false;
      }
    }
    return true;
    /*
    ++this->hop;
    if(this->hop >= st.Hops(car)) {
      this->hop = 1;
      ++this->car;
      if(this->car >= st.in.Cars()) {
        return false;
      }
    }
    return true;
    */
  }
};

class TOP_MoveSwap {
    friend bool operator==(const TOP_MoveSwap&, const TOP_MoveSwap&);
    friend bool operator!=(const TOP_MoveSwap&, const TOP_MoveSwap&);
    friend bool operator<(const TOP_MoveSwap&, const TOP_MoveSwap&);
    friend std::ostream& operator<<(ostream&, const TOP_MoveSwap&);
    friend std::istream& operator>>(istream&, TOP_MoveSwap&);
  public:
    TOP_MoveSwap();
    bool Feasible(const TOP_State&) const;
    bool ApplyTo(TOP_State&) const;
    
    int DeltaCostProfit_Swap(const TOP_State& st) const;
    int DeltaCostCar_Swap(const TOP_State& st) const;
    // int DeltaCostW_Swap(const TOP_State& st) const;

    // @TODO: Extra move fields
    carHop_s first, second;
    idx_t point;
};

// Do not change below this line

// Delta costs
class TOP_MoveSwapDeltaCostProfit_Swap : public DeltaCostComponent<TOP_Input, TOP_State, TOP_MoveSwap, int> {
  public:
    TOP_MoveSwapDeltaCostProfit_Swap(const TOP_Input & in, TOP_CostProfit_Swap& cc)
     : DeltaCostComponent<TOP_Input, TOP_State, TOP_MoveSwap, int>(in, cc, "TOP_MoveSwapDeltaCostProfit_Swap") {}
    int ComputeDeltaCost(const TOP_State& st, const TOP_MoveSwap& m) const { return m.DeltaCostProfit_Swap(st); }
};

class TOP_MoveSwapDeltaCostCar_Swap : public DeltaCostComponent<TOP_Input, TOP_State, TOP_MoveSwap, int> {
  public:
    TOP_MoveSwapDeltaCostCar_Swap(const TOP_Input & in, TOP_CostCar_Swap& cc)
     : DeltaCostComponent<TOP_Input, TOP_State, TOP_MoveSwap, int>(in, cc, "TOP_MoveSwapDeltaCostCar_Swap") {}
    int ComputeDeltaCost(const TOP_State& st, const TOP_MoveSwap& m) const { return m.DeltaCostCar_Swap(st); }
};

// class TOP_MoveSwapDeltaCostW_Swap : public DeltaCostComponent<TOP_Input, TOP_State, TOP_MoveSwap, int> {
//   public:
//     TOP_MoveSwapDeltaCostW_Swap(const TOP_Input & in, TOP_CostW_Swap& cc)
//      : DeltaCostComponent<TOP_Input, TOP_State, TOP_MoveSwap, int>(in, cc, "TOP_MoveSwapDeltaCostW_Swap") {}
//     int ComputeDeltaCost(const TOP_State& st, const TOP_MoveSwap& m) const { return m.DeltaCostW_Swap(st); }
// };


// Explorer

class TOP_MoveSwapNeighborhoodExplorer
 : public NeighborhoodExplorer<TOP_Input, TOP_State, TOP_MoveSwap>
{
  public:
    TOP_MoveSwapNeighborhoodExplorer(const TOP_Input& pin, StateManager<TOP_Input, TOP_State>& psm, TOP_CostContainer& cc)
     : NeighborhoodExplorer<TOP_Input, TOP_State, TOP_MoveSwap>(pin, psm, "TOP_MoveNeighborhoodExplorer")
     , _Car_Swap(in, cc._Car_Swap), _Profit_Swap(in, cc._Profit_Swap) /*, _W_Swap(in, cc._W_Swap)*/ {
      AddDeltaCostComponent(_Car_Swap);
      AddDeltaCostComponent(_Profit_Swap);
      //AddDeltaCostComponent(_W_Swap);
    }
    void RandomMove(const TOP_State&, TOP_MoveSwap&) const;
    void FirstMove(const TOP_State&, TOP_MoveSwap&) const;
    bool NextMove(const TOP_State&, TOP_MoveSwap&) const;
    bool FeasibleMove(const TOP_State& st, const TOP_MoveSwap& m) const { return m.Feasible(st); }
    void MakeMove(TOP_State& st, const TOP_MoveSwap& m) const { if(!m.ApplyTo(st)) { throw runtime_error("Invalid move applied"); } }
  protected:
    TOP_MoveSwapDeltaCostCar_Swap _Car_Swap;
    TOP_MoveSwapDeltaCostProfit_Swap _Profit_Swap;
    // TOP_MoveSwapDeltaCostW_Swap _W_Swap;
};

#endif
