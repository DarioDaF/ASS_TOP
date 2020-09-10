import os.path as path
import os, json

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

conf = {}
with open('MoveCosts.json', 'r') as fp:
  conf = json.load(fp)

for move in conf['moves']:
  fnHpp = f'Moves/{move}.hpp'
  fnCpp = f'Moves/{move}.cpp'
  if path.exists(fnHpp) or path.exists(fnCpp):
    print(f'Mossa "{move}" già presente: non creata')
    continue
  with open(fnHpp, 'w') as hpp:
    hpp.write(f'''
#ifndef TOP_MOVE_{move.upper()}_HPP
#define TOP_MOVE_{move.upper()}_HPP

#include "../../common/TOP_Data.hpp"
#include "../TOP_Basics.hpp"
#include "../TOP_Costs.hpp"
#include <easylocal.hh>

using namespace EasyLocal::Core;

// Move

class TOP_Move{move} {{
    friend bool operator==(const TOP_Move{move}&, const TOP_Move{move}&);
    friend bool operator!=(const TOP_Move{move}&, const TOP_Move{move}&);
    friend bool operator<(const TOP_Move{move}&, const TOP_Move{move}&);
    friend std::ostream& operator<<(ostream&, const TOP_Move{move}&);
    friend std::istream& operator>>(istream&, TOP_Move{move}&);
  public:
    TOP_Move{move}() {{}}
    bool Feasible(const TOP_State&) const;
    bool ApplyTo(TOP_State&) const;
''')
    for cost in conf['costs']:
      hpp.write(f'''
    {cost['type']} DeltaCost{cost['name']}(const TOP_State& st) const;
''')
    hpp.write(f'''
    // @TODO: Extra move fields
}};

// Do not change below this line

// Delta costs

''')
    for cost in conf['costs']:
      hpp.write(f'''
class TOP_Move{move}DeltaCost{cost['name']} : public DeltaCostComponent<TOP_Input, TOP_State, TOP_Move{move}, {cost['type']}> {{
  public:
    TOP_Move{move}DeltaCost{cost['name']}(const TOP_Input & in, TOP_Cost{cost['name']}& cc)
     : DeltaCostComponent<TOP_Input, TOP_State, TOP_Move{move}, {cost['type']}>(in, cc, "TOP_Move{move}DeltaCost{cost['name']}") {{}}
    {cost['type']} ComputeDeltaCost(const TOP_State& st, const TOP_Move{move}& m) const {{ return m.DeltaCost{cost['name']}(st); }}
}};
''')
    hpp.write(f'''

// Explorer
''')
    costInits = ', '.join([ f'''_{cost['name']}(in, cc._{cost['name']})''' for cost in conf['costs'] ])
    hpp.write(f'''
class TOP_Move{move}NeighborhoodExplorer
 : public NeighborhoodExplorer<TOP_Input, TOP_State, TOP_Move{move}>
{{
  public:
    TOP_Move{move}NeighborhoodExplorer(const TOP_Input& pin, StateManager<TOP_Input, TOP_State>& psm, TOP_CostContainer& cc)
     : NeighborhoodExplorer<TOP_Input, TOP_State, TOP_Move{move}>(pin, psm, "TOP_MoveNeighborhoodExplorer")
     , {costInits} {{
''')
    for cost in conf['costs']:
      hpp.write(f'''
      AddDeltaCostComponent(_{cost['name']});
''')
    hpp.write(f'''
    }}
    void RandomMove(const TOP_State&, TOP_Move{move}&) const;
    void FirstMove(const TOP_State&, TOP_Move{move}&) const;
    bool NextMove(const TOP_State&, TOP_Move{move}&) const;
    bool FeasibleMove(const TOP_State& st, const TOP_Move{move}& m) const {{ return m.Feasible(st); }}
    void MakeMove(TOP_State& st, const TOP_Move{move}& m) const {{ if(!m.ApplyTo(st)) {{ throw runtime_error("Invalid move applied"); }} }}
  protected:
''')
    for cost in conf['costs']:
      hpp.write(f'''
    TOP_Move{move}DeltaCost{cost['name']} _{cost['name']};
''')
    hpp.write(f'''

}};

#endif
''')

  with open(fnCpp, 'w') as cpp:
    cpp.write(f'''
#include "{move}.hpp"

// Move

bool operator==(const TOP_Move{move}& m1, const TOP_Move{move}& m2) {{
  // @TODO: Compare m1 and m2 fields
  throw std::logic_error("TOP_Move{move}: cannot compare");
}}
bool operator!=(const TOP_Move{move}& m1, const TOP_Move{move}& m2) {{
  return !(m1 == m2); // Stubbed
}}
bool operator<(const TOP_Move{move}& m1, const TOP_Move{move}& m2) {{
  // @TODO: Total order of move
  throw std::logic_error("TOP_Move{move}: cannot order");
}}

std::ostream& operator<<(ostream& os, const TOP_Move{move}& m) {{
  // @TODO: Print move
  return os;
}}
std::istream& operator>>(istream& is, TOP_Move{move}& m) {{
  // @TODO: Load move from stream
  throw std::logic_error("TOP_Move{{move}}: Cannot load from stream");
  return is;
}}

bool TOP_Move{move}::Feasible(const TOP_State& st) const {{
  // @TODO: Return if the move is feasible from state st
  throw std::logic_error("TOP_Move{move}::Feasible not implemented");
}}

bool TOP_Move{move}::ApplyTo(TOP_State& st) const {{
  // @TODO: Apply the move to st
  throw std::logic_error("TOP_Move{move}::ApplyTo not implemented");
}}

''')
    for cost in conf['costs']:
      cpp.write(f'''
{cost['type']} TOP_Move{move}::DeltaCost{cost['name']}(const TOP_State& st) const {{
  // @TODO: Return delta cost
  throw std::logic_error("TOP_Move{move}::DeltaCost{cost['name']} not implemented");
  return 0;
}}
''')
    cpp.write(f'''

// Explorer

void TOP_Move{move}NeighborhoodExplorer::RandomMove(const TOP_State& st, TOP_Move{move}& m) const {{
  // @TODO: Set m to a random move in the neighborhood
  throw std::logic_error("TOP_Move{move}NeighborhoodExplorer::RandomMove not implemented");
}}

void TOP_Move{move}NeighborhoodExplorer::FirstMove(const TOP_State& st, TOP_Move{move}& m) const {{
  // @TODO: Set m to the first move, always called before NextMove
  // (save order information inside m because this class is immutable/const)
  throw std::logic_error("TOP_Move{move}NeighborhoodExplorer::FirstMove not implemented");
}}

bool TOP_Move{move}NeighborhoodExplorer::NextMove(const TOP_State& st, TOP_Move{move}& m) const {{
  // @TODO: Use m and st to find next move
  // Any state should be reachable from any other with some sequence of moves
  throw std::logic_error("TOP_Move{move}NeighborhoodExplorer::NextMove not implemented");
}}
''')

with open('TOP_Costs.hpp', 'w') as hpp:
  hpp.write(f'''
#ifndef TOP_COSTS_HPP
#define TOP_COSTS_HPP

#include "TOP_Basics.hpp"
#include "TOP_Helpers.hpp"
#include <easylocal.hh>

using namespace EasyLocal::Core;
''')
  for cost in conf['costs']:
    hpp.write(f'''
class TOP_Cost{cost['name']}
 : public CostComponent<TOP_Input, TOP_State, {cost['type']}> {{
  public:
    TOP_Cost{cost['name']}(const TOP_Input& in, {cost['type']} w, bool hard)
     : CostComponent<TOP_Input, TOP_State, {cost['type']}>(in, w, hard, "TOP_Cost{cost['name']}") {{}}
    {cost['type']} ComputeCost(const TOP_State& st) const; // @TODO: Implement in cpp
    void PrintViolations(const TOP_State& st, ostream& os = cout) const; // @TODO: Implement in cpp
}};
''')
  costInits = ', '.join([ f'''_{cost['name']}(in, {cost['weight']}, {cost['hard'] and 'true' or 'false'})''' for cost in conf['costs'] ])
  hpp.write(f'''

// Do not change below this line

class TOP_CostContainer {{
  public:
    TOP_CostContainer(const TOP_Input& in)
     : {costInits} {{}}
    void AddCostComponents(TOP_StateManager& sm) {{
''')
  for cost in conf['costs']:
    hpp.write(f'''
      sm.AddCostComponent(_{cost['name']});
''')
  hpp.write(f'''
    }}
''')
  for cost in conf['costs']:
    hpp.write(f'''
    TOP_Cost{cost['name']} _{cost['name']};
''')
  hpp.write(f'''
}};

#endif
''')
