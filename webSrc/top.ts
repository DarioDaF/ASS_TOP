
export type TOPPoint = {
  x: number,
  y: number,
  profit: number
};
export type TOPHop = {
  car: number,
  point: number
};
export type TOPMap = {
  n: number;
  m: number;
  tmax: number;
  points: TOPPoint[];
  h: number;
  hops: TOPHop[];
}

export function parseTOPMap(s: string) {
  const lines = s.split('\n');
  const res = {
    n: parseInt(lines[0].split(' ')[1]),
    m: parseInt(lines[1].split(' ')[1]),
    tmax: parseFloat(lines[2].split(' ')[1]),
    points: [],
    h: 0,
    hops: []
  } as TOPMap;
  for(let i = 0; i < res.n; ++i) {
    const [ x, y, profit ] = lines[i + 3].split('\t');
    res.points.push({ x: parseFloat(x), y: parseFloat(y), profit: parseInt(profit) } as TOPPoint);
  }
  // Output can be missing
  if(lines.length > 3 + res.n) {
    const hStr = lines[3 + res.n].split(' ');
    if(hStr.length >= 2) {
      res.h = parseInt(hStr[1]);
      for(let i = 0; i < res.h; ++i) {
        const [ car, point ] = lines[i + 4 + res.n].split('\t');
        res.hops.push({ car: parseInt(car), point: parseInt(point) } as TOPHop);
      }
    }
  }
  return res;
}
