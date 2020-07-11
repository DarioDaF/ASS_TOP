import { toName, lerp, createSvg, emptyElement, createSvgArrow, createCompoundSlider } from './lib.js'; // No extension will fail in import on browser
// To use modules you need type="module" but this forces CORS checks...

import { parseTOPMap, TOPMap, TOPPoint } from './top.js';
import { apiGetList, apiSolve, apiInstance, ApiList } from './api.js';

const $fileName = document.getElementById('fileName');
const $mapCanvasContainer = document.getElementById('mapCanvasContainer');
const $bFirstThunder = document.getElementById('bFirstThunder');
const $bStartThunder = document.getElementById('bStartThunder');
const $bStopThunder = document.getElementById('bStopThunder');
const $selInstance = document.getElementById('selInstance') as HTMLSelectElement;
const $selSolver = document.getElementById('selSolver') as HTMLSelectElement;
const $bSolve = document.getElementById('bSolve');
const $cbAutoUpdate = document.getElementById('cbAutoUpdate') as HTMLInputElement;
const $divOptions = document.getElementById('divOptions');

const optionSliders = ['a', 'b', 'c', 'd', 'e', 'f'];
let options = {};
for(const sl of optionSliders) {
  const compoundSl = createCompoundSlider(sl, (value) => {
    options[sl] = value;
    if($cbAutoUpdate.checked) {
      $bSolve.click();
    }
  }, 0, 100, 0.1);
  $divOptions.append(compoundSl.$cont);
}

const fileList: Record<string, File> = {};
const relMargin = 0.015;
const relRadius = 0.01;
const relWidth = 0.01;
const carColours = [
  "#75FF33", "#FF5733", "#33FFBD", "#DBFF33"
];

function pointColor(relP: number) {
  const col0 = { r: 1, g: 0, b: 0 };
  const col1 = { r: 0, g: 0, b: 1 };
  return toName(lerp(relP, col0, col1));
}

function drawMap(map: TOPMap) {
  // Compute bounding box and scale
  const bb = { minX: map.points[0].x, maxX: map.points[0].x, minY: map.points[0].y, maxY: map.points[0].y };
  for(const p of map.points) {
    bb.minX = Math.min(bb.minX, p.x);
    bb.minY = Math.min(bb.minY, p.y);
    bb.maxX = Math.max(bb.maxX, p.x);
    bb.maxY = Math.max(bb.maxY, p.y);
  }
  const size = { w: bb.maxX - bb.minX, h: bb.maxY - bb.minX };
  const genSize = Math.min(size.w, size.h);

  // Create static content
  const $svg = createSvg('svg', {
    width: '800',
    height: '600',
    viewBox: `${bb.minX - genSize * relMargin} ${bb.minY - genSize * relMargin} ${size.w + 2 * genSize * relMargin} ${size.h + 2 * genSize * relMargin}`,
    preserveAspectRatio: '',
    style: 'border: 1px solid black;'
  });
  const $bg = createSvg('rect', {
    x: bb.minX, y: bb.minY,
    width: size.w, height: size.h,
    fill: '#C0C0C0'
  });
  $svg.append($bg);

  // Draw hops
  const carPos = new Array(map.m).fill(0);
  for(const h of map.hops) {
    const oldPos = carPos[h.car];
    const newPos = h.point;

    const $hop = createSvgArrow('line', {
      x1: map.points[oldPos].x, y1: map.points[oldPos].y, x2: map.points[newPos].x, y2: map.points[newPos].y,
      stroke: carColours[h.car], 'stroke-width': genSize * relWidth
    }, $svg);
    $svg.append($hop);
    carPos[h.car] = h.point;
  }
  for(let c = 0; c < map.m; ++c) {
    const oldPos = carPos[c];
    const newPos = map.n - 1;
    const $hop = createSvgArrow('line', {
      x1: map.points[oldPos].x, y1: map.points[oldPos].y, x2: map.points[newPos].x, y2: map.points[newPos].y,
      stroke: carColours[c], 'stroke-width': genSize * relWidth
    }, $svg);
    $svg.append($hop);
  }

  // Draw points
  let maxP = 0;
  for(const p of map.points) {
    if(p.profit > maxP) {
      maxP = p.profit;
    }
  }
  for(const p_idx in map.points) {
    const p = map.points[p_idx];
    const $point = createSvg('circle', {
      cx: p.x, cy: p.y, r: genSize * relRadius,
      fill: pointColor(p.profit / maxP)
    });
    const $title = createSvg('title', {});
    $title.textContent = 'P' + p_idx;
    $point.append($title);
    $svg.append($point);
  }

  return $svg;
}

let currentMap: TOPMap = undefined;

/*
async function itemClick(e: MouseEvent) {
  const $this = e.target as HTMLElement;
  e.preventDefault();

  const { fileName } = $this.dataset;
  $fileName.innerText = fileName;
  const content = await fileList[fileName].text();
  const map = parseTOPMap(content);
  currentMap = map;
  console.log(map);
  const $svg = drawMap(map);
  emptyElement($mapCanvasContainer);
  $mapCanvasContainer.append($svg);
}

function updateFileList() {
  emptyElement($fileList);
  for(const [fileName, file] of Object.entries(fileList)) {
    const $li = document.createElement('li');
    const $a = document.createElement('a');
    $a.innerText = fileName;
    $a.dataset.fileName = fileName;
    $a.href = "#";
    $a.addEventListener('click', itemClick);
    $li.append($a);
    $fileList.append($li);
  }
}

$fileLoader.addEventListener('change', (e) => {
  const files = (event.target as HTMLInputElement).files;
  for(const file of Array.from(files)) {
    fileList[file.name] = file;
  }
  updateFileList();
});
*/

let list: ApiList = {
  "instances": [],
  "solvers": []
};

(async() => {
  list = await apiGetList();
  for(const inst of list.instances.sort()) {
    const $opt = document.createElement('option');
    $opt.value = inst;
    $opt.innerText = inst;
    $selInstance.append($opt);
  }
  for(const solver in list.solvers) {
    const $opt = document.createElement('option');
    $opt.value = solver;
    $opt.innerText = list.solvers[solver];
    $selSolver.append($opt);
  }
})();
$bSolve.addEventListener('click', async() => {
  const solver = $selSolver.value;
  const inst = $selInstance.value;

  const name = `${inst} (${solver === '' ? 'Input' : list.solvers[solver]})`

  emptyElement($mapCanvasContainer);
  $fileName.innerText = name + '...';

  let content = '';
  if(solver === '') {
    // Show input
    content = await apiInstance(inst);
  } else {
    const res = await apiSolve(inst, solver === '' ? undefined : parseInt(solver), options);
    console.log(JSON.stringify(res));
    content = res.solution;
    console.log(`Solution found with profit: ${res.profit}`);
  }

  const map = parseTOPMap(content);
  currentMap = map;
  console.log(map);
  const $svg = drawMap(map);
  $mapCanvasContainer.append($svg);

  $fileName.innerText = name;
});

// THUNDER solver

type Point = {
  x: number;
  y: number;
}
type ThunderData = {
  visited: boolean[],
  carPartialPos: Point[],
  carActPos: Point[],
  carStallIterations: number[],
  stallIterations: number
}

function pDist(p1: Point, p2: Point) {
  return Math.sqrt((p1.x - p2.x) ** 2 + (p1.y - p2.y) ** 2);
}
function pDistSq(p1: Point, p2: Point) {
  return (p1.x - p2.x) ** 2 + (p1.y - p2.y) ** 2;
}
function pLen(p: Point) {
  return Math.sqrt(p.x ** 2 + p.y ** 2);
}
function pLenSq(p: Point) {
  return p.x ** 2 + p.y ** 2;
}
function pAdd(p1: Point, p2: Point) {
  return { x: p1.x + p2.x, y: p1.y + p2.y };
}
function pSub(p1: Point, p2: Point) {
  return { x: p1.x - p2.x, y: p1.y - p2.y };
}
function pMul(p: Point, m: number) {
  return { x: p.x * m, y: p.y * m };
}
function pNorm(p: Point) {
  return pMul(p, 1/pLen(p));
}

function FirstThunder(map: TOPMap): ThunderData {
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

function NextThunder(map: TOPMap, thunder: ThunderData, K: number, dt: number, rangeSq: number, speedCap: number, useStall: boolean) {
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

function drawMapWithThunder(map: TOPMap, thunder: ThunderData) {
  const $svg = drawMap(map);

  // Compute bounding box and scale
  const bb = { minX: map.points[0].x, maxX: map.points[0].x, minY: map.points[0].y, maxY: map.points[0].y };
  for(const p of map.points) {
    bb.minX = Math.min(bb.minX, p.x);
    bb.minY = Math.min(bb.minY, p.y);
    bb.maxX = Math.max(bb.maxX, p.x);
    bb.maxY = Math.max(bb.maxY, p.y);
  }
  const size = { w: bb.maxX - bb.minX, h: bb.maxY - bb.minX };
  const genSize = Math.min(size.w, size.h);

  // Draw actual car position
  for(let c = 0; c < map.m; ++c) {
    const $point = createSvg('circle', {
      cx: thunder.carPartialPos[c].x, cy: thunder.carPartialPos[c].y, r: genSize * relRadius,
      fill: carColours[c]
    });
    $svg.append($point);
  }

  return $svg;
}

let currentThunder: ThunderData = undefined;

$bFirstThunder.addEventListener('click', () => {
  currentMap.hops = [];
  currentThunder = FirstThunder(currentMap);
  console.log(currentThunder);
  const $svg = drawMapWithThunder(currentMap, currentThunder);
  emptyElement($mapCanvasContainer);
  $mapCanvasContainer.append($svg);
});

let thunderInterval = -1;

function doThunder() {
  const motionDist = Math.max(0.2, (currentMap.tmax / 100));
  NextThunder(currentMap, currentThunder, 3, 0.01, motionDist ** 2, motionDist, true);
  const $svg = drawMapWithThunder(currentMap, currentThunder);
  emptyElement($mapCanvasContainer);
  $mapCanvasContainer.append($svg);
}

$bStartThunder.addEventListener('click', () => {
  if(thunderInterval < 0) {
    thunderInterval = setInterval(doThunder, 20);
  }
});

$bStopThunder.addEventListener('click', () => {
  clearInterval(thunderInterval);
  thunderInterval = -1;
});
