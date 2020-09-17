#ifndef TOP_BACKTRACKING_HPP
#define TOP_BACKTRACKING_HPP

#include "Backtracking.hpp"
#include "../common/TOP_Data.hpp"

typedef int cost_t;

/******************
 * Implementation *
 ******************/

/**
 * Class that represent the state and extend the concept of output
 */
class TOP_NodeOutput : public NodeOutput {
  public:
    TOP_NodeOutput& operator=(const TOP_Output& out) { // Opertaor= 
      this->out = new TOP_Output(out);
      return *this;
    }
    TOP_Output Output() const { return *out; }
    TOP_NodeOutput() : out(nullptr) {} // Constructor
    ~TOP_NodeOutput() { if(out != nullptr) { delete out; out = nullptr; } } // Destructor

  protected:
    std::ostream& print(std::ostream& os) const { // Print the output
      if(out != nullptr) {
        return os << *out;
      } else {
        return os << "()";
      }
    }

  private:
    TOP_Output* out = nullptr; 
};

/**
 * Class that represent the output and some useful functions to manage it in the backtracking 
 * algorithm. It contains also the function to evaluate the "non-improuving branch" condition  
 */
class TOP_Node : public Node<cost_t, TOP_NodeOutput>, public TOP_Output {
  public:
    using TOP_Output::TOP_Output; //TOP_Node(const TOP_Input& in) : TOP_Output(in) {}
    
    // operator TOP_NodeOutput() const {
    //   std::cerr << "Try2" << std::endl;
    //   TOP_NodeOutput nodeOutput { *this };
    //   return nodeOutput; // Dovrebbe tornare la copia sola lettura senza info duplicate di se stesso
    // }
    
    bool IsFeasible() const { return Feasible(); }; // Return if the output is feasible
    cost_t GetCost() const { return -PointProfit(); }; // Return the actual Profit of the output

    /**
     * Function that allow to estimate the possible amount of profit that can be done
     * from the insertion of one point. It takes all the point into the ellipse which 
     * area is drawn by the remaining travel time of all the cars. The sum of those 
     * profits allow to understand the "Non improuving branch" condition
     * 
     * @return the sum of the points profit in the ellipse
     */
    cost_t GetMinCost() const;
};

/**
 * Class that represent the wallker along the tree of solutions, both partial and total ones. Because
 * of the prior non-knowledge og the lenght of one branch, the class is provided by various function
 * that allow to walk from one state to the other, included the empty solution
 */
class TOP_Walker : public TreeWalker<TOP_Node> {
  public:
    TOP_Walker(const TOP_Input& in, double wProfit, double wTime, double maxDeviation, double wNonCost)
      : TreeWalker(TOP_Node(in)), in(in), wProfit(wProfit), wTime(wTime), maxDeviation(maxDeviation), wNonCost(wNonCost) {} // Constructor

    void GoToRoot() { // Empty solution  and Clear solution 
      current = TOP_Node(in); 
      carAssignmentOrder.clear();
    }

    // Walker functions

    /**
     * Walker function that allow to go down to the child of one branch. In particular
     * takes the current state of the problem and insert the next point to its car. The 
     * sequence in which the couple is choosen is determinated by the rating of the point
     * 
     * @return true if there is one child to go to, false otherwise
     */
    bool GoToChild();

    /**
     * Walker function that allow to go the sibiling on at one level. In particular takes 
     * the current state of the problem and swap the current inserted node with the next 
     * point to its car. The sequence in which the couple is choosen is determinated by the 
     * rating of the point.
     * 
     * @return true if there is one sibiling to go to, false otherwise and rollback to start the car
     */
    bool GoToSibiling();

    /**
     * Walker function that allow to go the parent level of one selected car. In particular takes 
     * the current state of the problem and remove the last point insertert into the last car that 
     * insert it. 
     * 
     * @return true if there is one parent to go to, false otherwise
     */
    bool GoToParent();

  private:
    const TOP_Input& in;
    std::vector<idx_t> carAssignmentOrder;
    double wProfit;
    double wTime;
    double maxDeviation;
    double wNonCost;
};

/**
 * Class that represent the checker of the consistency of the current state
 */
class TOP_Checker : public BoundChecker<TOP_Node> {
  public:
    void Reset() {
      bestCost = 0; // -profit
      bestNode = TOP_NodeOutput();
    }
    NodeBound CheckNode(const TOP_Node& n); // Check bounds and feasibility, can be used to update bounds
};

#endif
