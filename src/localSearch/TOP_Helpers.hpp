// File TOP_Helpers.hh
#ifndef TOP_HELPERS_HPP
#define TOP_HELPERS_HPP

#include "TOP_Basics.hpp"
#include <easylocal.hh>

using namespace EasyLocal::Core;

/***************************************************************************
 * State Manager 
 ***************************************************************************/

class TOP_StateManager : public StateManager<TOP_Input,TOP_State> 
{
public:
  // Constructor 
  TOP_StateManager(const TOP_Input &);

  /**
   * Initial state builder (random assignation of point to cars)
   *
   * @param st state of the current output 
   * @return [void]
   */
  void RandomState(TOP_State&);   

  /**
   * Verify if one state in consistent. It need to satisfy two conditions: the max travel time of 
   * each car should be less than the max time admitted and each poit should be visited at least one time.
   *
   * @param st state of the current output 
   * @return true if consistent, false otherwise
   */
  bool CheckConsistency(const TOP_State& st) const;
protected:
}; 

/***************************************************************************
 * Output Manager:
 ***************************************************************************/
class TOP_OutputManager
  : public OutputManager<TOP_Input,TOP_Output,TOP_State> 
{
public:
  TOP_OutputManager(const TOP_Input & pin)
    : OutputManager<TOP_Input,TOP_Output,TOP_State>(pin,"TOPOutputManager") {}
  void InputState(TOP_State&, const TOP_Output&) const;  
  void OutputState(const TOP_State&, TOP_Output&) const; 
};

#endif
