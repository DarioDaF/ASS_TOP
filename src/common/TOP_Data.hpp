#ifndef TOP_DATA_HPP
#define TOP_DATA_HPP

#include "Utils.hpp"

#include <iostream>
#include <vector>
#include <cmath>

// Functions inside class are inlined

/**
 * Input format:
 *  n {Points}
 *  m {Cars}
 *  tmax {MaxTime}
 *  {for Points}
 *    {x} '\t' {y} '\t' {profit}
 *  {end for}

 * Output format: (start and end points are implicit)
 *  h {Hops}
 *  {for Hops}
 *    {car} '\t' {dest}
 *  {end for}
 */
 
/**
 * Class that represent each point through its coordinates and profit. It also has 
 * simple functions necessary for the operations on the points (i.e. distance).
 */
class TOP_Point {
    friend std::ostream& operator<<(std::ostream& os, const TOP_Point& p);
    friend std::istream& operator>>(std::istream& is, TOP_Point& p);
  
  public:
    TOP_Point(double x, double y, int p) : x(x), y(y), p(p) {};
    TOP_Point() : TOP_Point(0, 0, 0) {};

    double X() const { return x; } // Coordinates
    double Y() const { return y; }
    int Profit() const { return p; }

    // Basic functions
    void Set(double x, double y, int p) { this->x = x; this->y = y; this->p = p; }
    void Add(const TOP_Point& other) { this->x += other.x; this->y += other.y; this->p += other.p; }
    void Divide(int n) { this->x /= n; this->y /= n; this->p /= n; }

    // Distance functions
    double DistanceSq(TOP_Point p2) const { return (this->x - p2.x)*(this->x - p2.x) + (this->y - p2.y)*(this->y - p2.y); }
    double Distance(TOP_Point p2) const { return sqrt(DistanceSq(p2)); }
    
  private:
    double x, y;
    int p;
};

/**
 * Class that represent the input read from the instance files. Contains information about the number 
 * of cars and points, about the maximum travel time available for all the cars, the start and the end 
 * point, the list of points and some useful functions to speed up the computation
 */
class TOP_Input {
    friend std::ostream& operator<<(std::ostream& os, const TOP_Input& in);
    friend std::istream& operator>>(std::istream& is, TOP_Input& in);
  
  public:
    std::string name;
    TOP_Input(const std::string& filename); // Input 
    TOP_Input() { Clear(); }
    void Clear();

    // Functions

    // Number of cars
    int Cars() const { return cars; } 

    // Number of points
    int Points() const { return points.size(); } 

    // Max travel time admitted
    double MaxTime() const { return max_time; } 

    // index point
    const TOP_Point& Point(idx_t pos) const { return points[pos]; }

    // Start point 
    idx_t StartPoint() const { return 0; }
    
    // End point (conventionally the last from instances file)
    idx_t EndPoint() const { return points.size() - 1; } 

    const double Distance(idx_t p1, idx_t p2) const { return Point(p1).Distance(Point(p2)); }

  private:
    int cars;
    double max_time;
    std::vector<TOP_Point> points;
};

/**
 * Class that represent the input read from the instance files. Contains information about the number 
 * of cars and points, about the maximum travel time available for all the cars, the start and the end 
 * point, the list of points and some useful functions to speed up the computation
 */
class TOP_Output {
    friend std::ostream& operator<<(std::ostream& os, const TOP_Output& out);
    friend std::istream& operator>>(std::istream& is, TOP_Output& out);
    friend bool operator==(const TOP_Output& out1, const TOP_Output& out2);
    
  public:
    // Helpful structs for the moves
    struct SimulateMoveCarResult { // Move simulation
      bool feasible;
      double extraTravelTime;
    };
    enum InsertMode { // Insertion move
      SIMULATE,
      NORMAL,
      FORCE
    };

    // Input constructor
    TOP_Output(const TOP_Input& in) : in(in),
      car_hops(in.Cars()), visited(in.Points()),
      travel_time(in.Cars()) { Clear(); }
    void Clear();

    // Output constructor
    TOP_Output(const TOP_Output& out) : in(out.in),
      car_hops(out.car_hops), visited(out.visited),
      travel_time(out.travel_time),
      point_profit(out.point_profit),
      time_violations(out.time_violations) {}

    // operator= overwrite two outputs
    TOP_Output& operator=(const TOP_Output& out) { 
      const_cast<TOP_Input&>(this->in) = out.in;
      this->car_hops = out.car_hops;
      this->visited = out.visited;
      this->travel_time = out.travel_time;
      this->point_profit = out.point_profit;
      this->time_violations = out.time_violations;
      return *this;
    }

    // Basic functions

    // return the travel time of one specified car
    double TravelTime(idx_t car) const { return travel_time[car]; }

    // Verify if one point is already visited 
    bool Visited(idx_t point) const { return visited[point] > 0; }

     // Verify if one point is already visited by two cars at the same time
    bool DoubleVisited(idx_t point) const { return visited[point] > 1; }  

    // Verify feasibility
    bool Feasible() const { return time_violations == 0; } 

     // Return the profit already gained
    int PointProfit() const { return point_profit; }

    // Return the hops made by one car
    int Hops(idx_t car) const { return car_hops[car].size() + 1; } 
    
    // Return the last point inserted from a specified car
    idx_t CarPoint(idx_t car) const { 
      return car_hops[car].empty() ? in.StartPoint() : car_hops[car].back(); 
    } 

    // Return the point index reached by the car in its one specific hop
    idx_t Hop(idx_t car, idx_t hop) const { 
      if(hop <= 0) return in.StartPoint();
      if(hop > car_hops[car].size()) return in.EndPoint();
      return car_hops[car][hop - 1];
    }
    
    // Move functions
    idx_t RollbackCar(idx_t car); 
    const SimulateMoveCarResult MoveCar(idx_t car, idx_t dest, bool force = true); 
    const SimulateMoveCarResult SimulateMoveCar(idx_t car, idx_t dest) const;
    const SimulateMoveCarResult InsertHop(idx_t car, idx_t hop, idx_t dest, InsertMode mode = FORCE);
    const SimulateMoveCarResult RemoveHop(idx_t car, idx_t hop);
    
    const TOP_Input& in;

  private:

    std::vector<std::vector<idx_t>> car_hops; // car_hops[car][hop] = point
    std::vector<int> visited; // Number of visits for each point
    
    // Redundant data
    std::vector<double> travel_time;
    int point_profit;
    int time_violations;
    
    // Internal functions
    void IncrementVisited(idx_t point, int count);
    void IncrementTravelTime(idx_t car, double count);
};

// Internals
double extraDistance(TOP_Point pStart, TOP_Point pNew, TOP_Point pEnd);

#endif
