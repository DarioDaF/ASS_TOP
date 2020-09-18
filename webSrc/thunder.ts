
import { TOPMap } from './top.js';

import { Point, pAdd, pSub, pLenSq, pMul, pNorm, pLen, pDist, pDistSq } from './lib.js';

export type ThunderData = {
  visited: boolean[],
  carPartialPos: Point[],
  carActPos: Point[],
  carStallIterations: number[],
  stallIterations: number
}

export function FirstThunder(map: TOPMap): ThunderData {
  // Prepare thunder
  const thunder = {
    carPartialPos: Array.from({ length: map.m }, e => ({ ...map.points[0] })),
    carActPos: Array.from({ length: map.m }, e => ({ ...map.points[0] })), // new Array(map.m).fill(map.points[0]) this copies reference
    carStallIterations: new Array(map.m).fill(0),
    stallIterations: 0,
    visited: new Array(map.n).fill(false),
  }
  thunder.visited[0] = true;
  thunder.visited[map.n - 1] = true; // Avoid end
  return thunder;
}

export function NextThunder(map: TOPMap, thunder: ThunderData, K: number, dt: number, rangeSq: number, speedCap: number, useStall: boolean) {
  ++thunder.stallIterations;
  // Start all cars and move to the electric field
  for(let c = 0; c < map.m; ++c) {
    if(useStall && thunder.carStallIterations[c] > 0) {
      --thunder.carStallIterations[c];
      continue; // Skip car form future
    }
    let force = { x: 0, y: 0 };
    for(let p = 0; p < map.n; ++p) {
      if(thunder.visited[p]) {
        continue;
      }
      const pDiff = pSub(map.points[p], thunder.carPartialPos[c]);
      const forceScalar = K * map.points[p].profit / pLenSq(pDiff);
      if(pLenSq(pDiff) <= rangeSq) {
        console.warn(`Error: uncaptured point, might diverge: (${pDiff.x}, ${pDiff.y})`);
      }
      force = pAdd(force, pMul(pNorm(pDiff), forceScalar));
    }
    // Move the car by force, limit max travel?
    let impulse = pMul(force, dt);
    if(pLen(impulse) > speedCap) {
      console.warn(`Velocity capped: (${force.x}, ${force.y})`);
      impulse = pMul(impulse, speedCap / pLen(impulse));
    }
    thunder.carPartialPos[c] = pAdd(thunder.carPartialPos[c], impulse);
    // Check if in points range
    for(let p = 0; p < map.n; ++p) {
      if(thunder.visited[p]) {
        continue;
      }
      if(pDistSq(map.points[p], thunder.carPartialPos[c]) <= rangeSq) {
        // Assign car and break
        // Only if feasible
        
        let travelTime = 0;
        {
          let lastPoint = map.points[0];
          for(const h of map.hops) {
            if(h.car != c) continue;
            const stepTime = pDist(lastPoint, map.points[h.point]);
            travelTime += stepTime;
            lastPoint = map.points[h.point];
          }
          {
            const stepTime = pDist(lastPoint, map.points[p]);
            travelTime += stepTime;
            lastPoint = map.points[p];
          }
          {
            const stepTime = pDist(lastPoint, map.points[map.n - 1]);
            travelTime += stepTime;
          }
        }
        if(travelTime <= map.tmax) {
          map.hops.push({ car: c, point: p });
          thunder.visited[p] = true;
          thunder.carActPos[c] = map.points[p];
          thunder.carStallIterations[c] += thunder.stallIterations;
          thunder.stallIterations = 0;
          console.log(thunder);
          for(let c = 0; c < map.m; ++c) {
            thunder.carPartialPos[c] = { ...thunder.carActPos[c] }; // Reset all cars (discharge!)
          }
          break;
        }
      }
    }
  }
}
