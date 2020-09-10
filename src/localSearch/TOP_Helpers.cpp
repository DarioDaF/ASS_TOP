// File TOP_Helpers.cc
#include "TOP_Helpers.hpp"

#include "../common/Utils.hpp"

// constructor
TOP_StateManager::TOP_StateManager(const TOP_Input & pin)
  : StateManager<TOP_Input,TOP_State>(pin, "TOPStateManager")  {}

// initial state builder (random rooms)
void TOP_StateManager::RandomState(TOP_State& st) {
  // Insert the code that creates a random state in object st

  // Assegna ciascun punto a una macchina o fuori
  // E lega una priorità per cui ordinare poi

/*
  std::vector<std::vector<idx_t>> assignments(st.in.Points());

  for(auto point : NumberRange(st.in.Points())) {
    idx_t car = Random::Uniform<idx_t>(-1, st.in.Cars() - 1);
    if(car >= 0) {
      assignments[car].emplace(assignments[car].begin() + Random::Uniform<idx_t>(0, assignments[car].size() - 1), point);
    }
  }
*/

  st.Clear();
  for(auto point : NumberRange<idx_t>(1, st.in.Points() - 1)) {
    idx_t car = Random::Uniform<idx_t>(-1, st.in.Cars() - 1);
    if(car >= 0) {
      st.InsertHop(car, Random::Uniform<idx_t>(1, st.Hops(car)), point);
    }
  }

	//throw logic_error("TOP_StateManager::RandomState not implemented yet");
} 

bool TOP_StateManager::CheckConsistency(const TOP_State& st) const {
  // Insert the code that checks if state in object st is consistent
  // (for debugging purposes)
	throw logic_error("TOP_StateManager::CheckConsistency not implemented yet");
  return true;
}

/*****************************************************************************
 * Output Manager Methods
 *****************************************************************************/

void TOP_OutputManager::InputState(TOP_State& st, const TOP_Output& out) const {
  st = out;
}

void TOP_OutputManager::OutputState(const TOP_State& st, TOP_Output& out) const {
  out = st;
}
