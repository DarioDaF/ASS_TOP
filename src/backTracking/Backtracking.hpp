#ifndef BACKTRACKING_HPP
#define BACKTRACKING_HPP

#include <iostream>
#include <chrono>
using namespace std::chrono;

/**
 * Class that represent the state and extend the concept of output
 */
class NodeOutput {
  protected:
    virtual std::ostream& print(std::ostream& os) const = 0;
  public:
    friend std::ostream& operator<<(std::ostream& os, const NodeOutput& node) {
      return node.print(os);
    }
};

/**
 * Class that represent the output (Node) and some useful functions to manage it in the backtracking 
 * algorithm. It contains also the function to evaluate the "non-improuving branch" condition. In our 
 * algorithm the state and the output are circa the same entity  
 */
template<typename _Cost, typename _Output>
class Node {
  public:
    typedef _Output Output;
    typedef _Cost Cost;
    //virtual operator Output() const = 0; // Convert to NodeOutput
    virtual bool IsFeasible() const { return true; };
    virtual Cost GetCost() const = 0; // Cost always to minimize (can be -profit)
};

/**
 * Enumeration variable used to check the current state of one solution, partial or total
 */
enum NodeBound {
  NB_Normal,
  NB_UnFeasible,
  NB_NonImproving
};

/**
 * Class that represent the checker of the consistency of the current state and also provided
 * some useful functions to update or clear the data of one node
 */
template<typename _Node>
class BoundChecker {
  public:
    virtual void Reset() = 0; /* {
      auto n = _Node(); // Not always present
      bestCost = n.GetCost();
      bestNode = n;
    } */
    virtual NodeBound CheckNode(const _Node& n) { // Check bounds and feasibility, can be used to update bounds
      // Be warned that the node can be unfeasible
      // Feasibility can be more strict due to bounds or learnt nogoods
      return n.IsFeasible() ? NB_Normal : NB_UnFeasible;
    };
    virtual bool UpdateBest(const _Node& n) { // Can be used to update bounds (but shouldn't) and return if best
      auto newCost = n.GetCost();
      //std::cerr << "Cost comparison: old " << bestCost << ", new " << newCost << std::endl;
      if(newCost < bestCost) {
        //std::cerr << "Saving new best..." << std::endl;
        bestNode = n; //typename _Node::Output(n); // Implicit cast
        bestCost = newCost;
        return true;
      }
      return false;
    }
    //bool IsNonImproving(const Node& n); // Can be not implemented
    //bool IsFeasible(const Node& n) { return n.IsFeasible(); }; // Can be not implemented
    // And be more restrictive due to learnt nogoods or something like that
    
    const typename _Node::Output& GetBest() const { return bestNode; }
    typename _Node::Cost GetBestCost() const { return bestCost; }

  protected:
    typename _Node::Output bestNode;
    typename _Node::Cost bestCost;
};

/**
 * Class that represent the wallker along the tree of solutions, both partial and total ones. Because
 * of the prior non-knowledge og the lenght of one branch, the class is provided by various function
 * that allow to walk from one state to the other, included the empty solution
 */
template<typename _Node>
class TreeWalker {
  protected:
    _Node current;
  public:
    TreeWalker(_Node current) : current(current) {}
    virtual void GoToRoot() = 0; // Resets walker
    virtual bool GoToChild(double wProfit, double wTime, double maxDeviation, double wNonCost) = 0; // Fails if leaf
    virtual bool GoToSibiling(double wProfit, double wTime, double maxDeviation, double wNonCost) = 0; // Fails if last sibiling, SOULD ALWAYS KEEP SAME DEPH
    virtual bool GoToParent() = 0; // Fails if root
    //bool IsRoot() const; // Can be not implemented
    //bool IsLeaf() const; // Can be not implemented
    const _Node& GetNode() const { return current; };
};

/**
 * Execute the Backtracking algorithm visiting all the solutions tree. This algorithm
 * is generalized because each branch can have different depth accordingly to the TOP 
 * problem beacuse of the different point choice during the execution of the algorithm. 
 * It also has a limit of time to resolve one instance: if the time take is higher than 
 * the max time permitted, the algorithm stop and save the best solution founded 
 * 
 * @param walker class that allow to walk into the solutions tree
 * @param checker class that allow to check the consistency of one solution
 * @param maxTime time max allow to the problem to solve one instance
 * @return void
 */
template<typename _Node>
void Backtrack(TreeWalker<_Node>& walker, BoundChecker<_Node>& checker, double maxTime, double wProfit, double wTime, double maxDeviation, double wNonCost) {
  checker.Reset();
  walker.GoToRoot();

  unsigned long count = 0;

  bool firstSolFound = false;
  bool searchEnded = false;
  bool backtrack = false;
  // std::cerr << "Started..." << std::endl;
  auto start = std::chrono::high_resolution_clock::now();

  while(!searchEnded) {
    if(!backtrack) {
      auto nodeBound = checker.CheckNode(walker.GetNode());
      if(firstSolFound) {
        backtrack = nodeBound != NodeBound::NB_Normal;
      } else {
        backtrack = nodeBound == NodeBound::NB_UnFeasible;
      }
    }
    if(backtrack) {
      //std::cerr << "Backtrack... ";
      //while(!walker.GoToSibiling() && walker.GoToParent());
      while(true) {
        if(walker.GoToSibiling(wProfit, wTime, maxDeviation, wNonCost)) {
          //std::cerr << "SIBILING" << std::endl;
          ++count;
          backtrack = false;
          break;
        } else {
          if(!walker.GoToParent()) {
            //std::cerr << "FAIL" << std::endl;
            searchEnded = true;
            break;
          } else {
            //std::cerr << "PARENT | ";
          }
        }
		  }
    } else {
      //std::cerr << "Descend... ";
      if(walker.GoToChild(wProfit, wTime, maxDeviation, wNonCost)) {
        // std::cerr << "OK" << std::endl;
        ++count;
      } else {
        // std::cerr << "LEAF" << std::endl;
        // std::cerr << "Leaf found" << std::endl;
        if(checker.UpdateBest(walker.GetNode())) {
          firstSolFound = true; // Should be even if not best?
          auto finish = std::chrono::high_resolution_clock::now();
          std::cerr << "New best solution " /*<< checker.GetBest()*/
                    << " (cost " << checker.GetBestCost() << ", iter " << count << " @" << duration_cast<duration<double>>(finish - start).count() << "s)" << std::endl;
        }
        backtrack = true; // Continue search
      }
    }
    auto timeStop = high_resolution_clock::now();
    if(duration_cast<duration<double>>(timeStop - start).count() >= maxTime) {
      std::cerr << "Cannot complete after " << duration_cast<duration<double>>(timeStop - start).count() << "s" << std::endl;
      return;
    }
  }
  auto finish = high_resolution_clock::now();
  std::cerr << "Completed after " << count << " iterations @" << duration_cast<duration<double>>(finish - start).count() << "s" << std::endl;
}

#endif
