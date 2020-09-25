#include "Swap.hpp"

using EasyLocal::Core::Random;

// Move

TOP_MoveSwap::TOP_MoveSwap() {}

bool operator==(const TOP_MoveSwap& m1, const TOP_MoveSwap& m2) {
  // @TODO: Compare m1 and m2 fields
  return m1.first == m2.first && m1.second == m2.second && m1.point == m2.point;
}

bool operator!=(const TOP_MoveSwap& m1, const TOP_MoveSwap& m2) {
  return !(m1 == m2); // Stubbed
}

bool operator<(const TOP_MoveSwap& m1, const TOP_MoveSwap& m2) {
  // @TODO: Compare m1 and m2 fields in order to verify which one is first

  return
    m1.first < m2.first || (m1.first == m2.first && (
      m1.second < m2.second || (m1.second == m2.second && (
        m1.point < m2.point
      ))
    ));
}

std::ostream& operator<<(ostream& os, const TOP_MoveSwap& m) {
  // @TODO: Print move
  if(m.first.IsValid()) {
    if(m.second.IsValid()) {
      os << "Swap " << m.first << " with " << m.second;
    } else {
      os << "Remove " << m.first;
    }
  } else {
    os << "Insert " << m.point << " into " << m.second;
  }
  return os;
}

std::istream& operator>>(istream& is, TOP_MoveSwap& m) {
  // @TODO: Load move from stream
  throw logic_error("TOP_MoveSwap: operator>> not implemented");
  return is;
}

bool TOP_MoveSwap::Feasible(const TOP_State& st) const {
  // @TODO: Return if the move is feasible from state st

  return true;

  //throw logic_error("TOP_MoveSwap::Feasible not implemented");

  /*
  if(DeltaCostCar_Swap(st) > 0) {
    return false;
  }
  return true;
  */
}

bool TOP_MoveSwap::ApplyTo(TOP_State& st) const {
  // @TODO: Apply the move to st
  if(second.IsValid()) {
    if(first.IsValid()) {
      // Swap (can be optimized later...) (WARNING, can be both on same car!, mind removing/inserting order)
      idx_t point1 = st.Hop(first.car, first.hop);
      idx_t point2 = st.Hop(second.car, second.hop);
      st.RemoveHop(first.car, first.hop);
      st.InsertHop(first.car, first.hop, point2);
      st.RemoveHop(second.car, second.hop);
      st.InsertHop(second.car, second.hop, point1);
    } else {
      // Insert
      st.InsertHop(second.car, second.hop, point);
    }
  } else {
    // Remove
    st.RemoveHop(first.car, first.hop);
  }

  return true;
}

int TOP_MoveSwap::DeltaCostProfit_Swap(const TOP_State& st) const {
  // @TODO: Return delta cost (negative profit!)
  
  if(second.IsValid()) {
    if(first.IsValid()) {
      // Swap
      return 0;
    } else {
      // Insert
      return -st.in.Point(point).Profit();
    }
  } else {
    // Remove
    return +st.in.Point(st.Hop(first.car, first.hop)).Profit();
  }
}

int TOP_MoveSwap::DeltaCostCar_Swap(const TOP_State& st) const {
  // @TODO: Return delta cost

  if(second.IsValid()) {
    if(first.IsValid()) {
      // Swap

/*
      idx_t pFirstPre = st.Hop(first.car, first.hop - 1);
      idx_t pFirst = st.Hop(first.car, first.hop);
      idx_t pFirstPost = st.Hop(first.car, first.hop + 1);

      idx_t pSecondPre = st.Hop(second.car, second.hop - 1);
      idx_t pSecond = st.Hop(second.car, second.hop);
      idx_t pSecondPost = st.Hop(second.car, second.hop + 1);
*/

      // @TODO: Fix better!!!
      if(first.car == second.car) {
        double timeDelta;
        if(first.hop + 1 == second.hop) {
          // A -> first -> second -> B to
          // A -> second -> first -> B

          // Delete A first and second B
          // Add A second and first B
          timeDelta =
            + st.in.Distance(st.Hop(first.car, first.hop - 1), st.Hop(second.car, second.hop))
            + st.in.Distance(st.Hop(first.car, first.hop), st.Hop(second.car, second.hop + 1))
            - st.in.Distance(st.Hop(first.car, first.hop - 1), st.Hop(first.car, first.hop))
            - st.in.Distance(st.Hop(second.car, second.hop), st.Hop(second.car, second.hop + 1));
        } else if(first.hop == second.hop + 1) {
          // A -> second -> first -> B to
          // A -> first -> second -> B

          // Delete A second and first B
          // Add A first and second B
          timeDelta =
            - st.in.Distance(st.Hop(first.car, first.hop - 1), st.Hop(second.car, second.hop))
            - st.in.Distance(st.Hop(first.car, first.hop), st.Hop(second.car, second.hop + 1))
            + st.in.Distance(st.Hop(first.car, first.hop - 1), st.Hop(first.car, first.hop))
            + st.in.Distance(st.Hop(second.car, second.hop), st.Hop(second.car, second.hop + 1));
        } else {
          // Same as below but summed

          timeDelta =
            - st.in.Distance(st.Hop(first.car, first.hop - 1), st.Hop(first.car, first.hop)) // Remove
            - st.in.Distance(st.Hop(first.car, first.hop), st.Hop(first.car, first.hop + 1))
            + st.in.Distance(st.Hop(first.car, first.hop - 1), st.Hop(second.car, second.hop)) // Insert
            + st.in.Distance(st.Hop(second.car, second.hop), st.Hop(first.car, first.hop + 1))
            - st.in.Distance(st.Hop(second.car, second.hop - 1), st.Hop(second.car, second.hop)) // Remove
            - st.in.Distance(st.Hop(second.car, second.hop), st.Hop(second.car, second.hop + 1))
            + st.in.Distance(st.Hop(second.car, second.hop - 1), st.Hop(first.car, first.hop)) // Insert
            + st.in.Distance(st.Hop(first.car, first.hop), st.Hop(second.car, second.hop + 1));
        }
        return
          (st.TravelTime(first.car) > st.in.MaxTime() ? -1 : 0) +
          (st.TravelTime(first.car) + timeDelta > st.in.MaxTime() ? +1 : 0);
      }

      double timeDeltaFirst =
        - st.in.Distance(st.Hop(first.car, first.hop - 1), st.Hop(first.car, first.hop)) // Remove
        - st.in.Distance(st.Hop(first.car, first.hop), st.Hop(first.car, first.hop + 1))
        + st.in.Distance(st.Hop(first.car, first.hop - 1), st.Hop(second.car, second.hop)) // Insert
        + st.in.Distance(st.Hop(second.car, second.hop), st.Hop(first.car, first.hop + 1));
      double timeDeltaSecond =
        - st.in.Distance(st.Hop(second.car, second.hop - 1), st.Hop(second.car, second.hop)) // Remove
        - st.in.Distance(st.Hop(second.car, second.hop), st.Hop(second.car, second.hop + 1))
        + st.in.Distance(st.Hop(second.car, second.hop - 1), st.Hop(first.car, first.hop)) // Insert
        + st.in.Distance(st.Hop(first.car, first.hop), st.Hop(second.car, second.hop + 1));
      return
        (st.TravelTime(first.car) > st.in.MaxTime() ? -1 : 0) +
        (st.TravelTime(first.car) + timeDeltaFirst > st.in.MaxTime() ? +1 : 0) +
        (st.TravelTime(second.car) > st.in.MaxTime() ? -1 : 0) +
        (st.TravelTime(second.car) + timeDeltaSecond > st.in.MaxTime() ? +1 : 0);
    } else {
      // Insert
      double timeDelta =
        + st.in.Distance(st.Hop(second.car, second.hop - 1), point)
        + st.in.Distance(point, st.Hop(second.car, second.hop))
        - st.in.Distance(st.Hop(second.car, second.hop - 1), st.Hop(second.car, second.hop));
      return
        (st.TravelTime(second.car) > st.in.MaxTime() ? -1 : 0) +
        (st.TravelTime(second.car) + timeDelta > st.in.MaxTime() ? +1 : 0);
    }
  } else {
    // Remove
    double timeDelta =
      - st.in.Distance(st.Hop(first.car, first.hop - 1), st.Hop(first.car, first.hop))
      - st.in.Distance(st.Hop(first.car, first.hop), st.Hop(first.car, first.hop + 1))
      + st.in.Distance(st.Hop(first.car, first.hop - 1), st.Hop(first.car, first.hop + 1));
    return
      (st.TravelTime(first.car) > st.in.MaxTime() ? -1 : 0) +
      (st.TravelTime(first.car) + timeDelta > st.in.MaxTime() ? +1 : 0);
  }

}

// int TOP_MoveSwap::DeltaCostW_Swap(const TOP_State& st) const {
//   // @TODO: Return delta cost
//   throw std::logic_error("TOP_MoveSwap::DeltaCostW_Swap not implemented");
//   return 0;
// }

// Explorer

bool FirstSwap(const TOP_State& st, TOP_MoveSwap& m) {
  //std::cout << "FirstSwap" << std::endl;
  m.point = 0; // Unused
  m.first = { .car = 0, .hop = 0 };
  if(!m.first.Increment(st)) {
    return false;
  }
  m.second = m.first;
  if(!m.second.Increment(st)) {
    return false;
  }
  return true;
}

bool NextSwap(const TOP_State& st, TOP_MoveSwap& m) {
  //std::cout << "NextSwap" << std::endl;
  if(!m.second.Increment(st)) {
    if(!m.first.Increment(st)) {
      return false;
    }
    m.second = m.first;
    if(!m.second.Increment(st)) {
      return false;
    }
  }
  return true;
}

bool FirstInsert(const TOP_State& st, TOP_MoveSwap& m) {
  //std::cout << "FirstInsert" << std::endl;
  m.first = { .car = 0, .hop = 0 }; // Unused invalid for insert
  m.second = { .car = 0, .hop = 0 };
  if(!m.second.Increment(st, true)) {
    return false;
  }
  do {
    ++m.point;
    if(m.point < st.in.Points() - 1) {
      if(!st.Visited(m.point)) {
        break;
      }
    } else {
      return false;
    }
  } while(true);
  return true;
}

bool NextInsert(const TOP_State& st, TOP_MoveSwap& m) {
  //std::cout << "NextInsert" << std::endl;
  if(!m.second.Increment(st, true)) {
    // Reset second
    m.second = { .car = 0, .hop = 0 };
    if(!m.second.Increment(st, true)) {
      return false;
    }
    // Increment point
    do {
      ++m.point;
    } while(m.point < st.in.Points() - 1 && st.Visited(m.point));

    if(m.point >= st.in.Points() - 1) {
      return false;
    }
  }
  return true;
}

bool FirstRemove(const TOP_State& st, TOP_MoveSwap& m) {
  //std::cout << "FirstRemove" << std::endl;
  m.second = { .car = 0, .hop = 0 }; // Unused
  m.point = 0; // Unused
  m.first = { .car = 0, .hop = 0 };
  if(!m.first.Increment(st)) {
    return false;
  }
  return true;
}

bool NextRemove(const TOP_State& st, TOP_MoveSwap& m) {
  //std::cout << "NextRemove" << std::endl;
  if(!m.first.Increment(st)) {
    return false;
  }
  return true;
}

void TOP_MoveSwapNeighborhoodExplorer::RandomMove(const TOP_State& st, TOP_MoveSwap& m) const {
  // @TODO: Set m to a random move in the neighborhood
  // Select random cars to swap

  while(true) {
    switch(Random::Uniform<int>(0, 2)) {
      case 0: {
        // Swap
        m.first.car = Random::Uniform<int>(0, st.in.Cars() - 1);
        if(st.Hops(m.first.car) <= 1) {
          continue;
        }
        m.first.hop = Random::Uniform<int>(1, st.Hops(m.first.car) - 1);

        m.second.car = Random::Uniform<int>(0, st.in.Cars() - 1);
        if(st.Hops(m.second.car) <= 1) {
          continue;
        }
        m.second.hop = Random::Uniform<int>(1, st.Hops(m.second.car) - 1);

        if(m.first == m.second) {
          continue;
        }

        m.point = 0;
        return;
      }
      case 1: {
        // Insert
        m.point = Random::Uniform<int>(1, st.in.Points() - 2);
        if(st.Visited(m.point)) {
          continue;
        }
        m.second.car = Random::Uniform<int>(0, st.in.Cars() - 1);
        m.second.hop = Random::Uniform<int>(1, st.Hops(m.second.car));

        m.first = { .car = 0, .hop = 0 };
        return;
      }
      case 2: {
        // Remove
        m.first.car = Random::Uniform<int>(0, st.in.Cars() - 1);
        if(st.Hops(m.first.car) <= 1) {
          continue;
        }
        m.first.hop = Random::Uniform<int>(1, st.Hops(m.first.car) - 1);

        m.second = { .car = 0, .hop = 0 };
        m.point = 0;
        return;
      }
    }
  }

/*
  // All insert and removes are st.Points() - 2
  // And the swaps are visited*(visited-1)/2

  idx_t visited = 0;
  for(auto point : NumberRange<idx_t>(st.in.Points())) {
    if(st.Visited(point)) {
      ++visited;
    }
  }
  idx_t swaps = visited * (visited - 1) / 2;

  idx_t nomralPoints = st.in.Points() - 2;

  idx_t moveIdx = Random::Uniform<idx_t>(1, nomralPoints + swaps);

  if(moveIdx <= nomralPoints) {
    if(st.Visited(moveIdx)) {
      // Remove
      m.point = 0;
      m.second = { .car = 0, .hop = 0 };
      // Find point
      for(auto car : NumberRange<id_t>(st.in.Cars())) {
        for(auto hop : NumberRange<id_t>(st.Hops(car))) {
          if(st.Hop(car, hop) == moveIdx) {
            m.first = { .car = car, .hop = hop };
            return;
          }
        }
      }
      throw logic_error("TOP_MoveSwapNeighborhoodExplorer::RandomMove visited point not present in cars");
    } else {
      // Insert
    }
  }
*/

  //throw logic_error("TOP_MoveSwapNeighborhoodExplorer::RandomMove not implemented");

}

void TOP_MoveSwapNeighborhoodExplorer::FirstMove(const TOP_State& st, TOP_MoveSwap& m) const {
  // @TODO: Set m to the first move, always called before NextMove
  // (save order information inside m because this class is immutable/const)

  if(!FirstSwap(st, m) && !FirstInsert(st, m) && !FirstRemove(st, m)) {
    throw logic_error("TOP_MoveSwapNeighborhoodExplorer::FirstMove unable to find first move");
  }

}


bool TOP_MoveSwapNeighborhoodExplorer::NextMove(const TOP_State& st, TOP_MoveSwap& m) const {
  // @TODO: Use m and st to find next move
  // Any state should be reachable from any other with some sequence of moves

  if(m.second.IsValid()) {
    if(m.first.IsValid()) {
      // Swap
      if(NextSwap(st, m)) {
        return true;
      }
      return FirstInsert(st, m) || FirstRemove(st, m);
    } else {
      // Insert
      if(NextInsert(st, m)) {
        return true;
      }
      return FirstRemove(st, m);
    }
  } else {
    // Remove
    return NextRemove(st, m);
  }

}
