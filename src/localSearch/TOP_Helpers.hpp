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
  TOP_StateManager(const TOP_Input &);
  void RandomState(TOP_State&);   
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
