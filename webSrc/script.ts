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
const $log = document.getElementById('log');
const $profit = document.getElementById('profit');

let options = {};

const fileList: Record<string, File> = {};
const relMargin = 0.015;
const relRadius = 0.01;
const relWidth = 0.01;
const carColours = [
  '#75FF33', '#FF5733', '#33FFBD', '#DBFF33'
];

function changeSolverOptions(solver) {
  options = {};
  emptyElement($divOptions);
  if(!(solver in list.solvers)) {
    return;
  }
  for(const param of list.solvers[solver].params) {
    options[param.name] = param.default;
    switch(param.type) {
      case 'bool': {
        const $label = document.createElement('label');
        $label.innerText = param.descr;
        const $check = document.createElement('input');
        $check.type = 'checkbox';
        $check.checked = param.default as boolean;

        const $div = document.createElement('div');
        $div.append($label);
        $div.append($check);

        $check.addEventListener('change', (e) => {
          options[param.name] = $check.checked;
          if($cbAutoUpdate.checked) {
            $bSolve.click();
          }
        });
        $divOptions.append($div);

        break;
      }
      case 'float': {
        const compoundSl = createCompoundSlider(param.descr, (value) => {
          options[param.name] = value;
          if($cbAutoUpdate.checked) {
            $bSolve.click();
          }
        }, param.min, param.max, 0.01, param.default as number);
        $divOptions.append(compoundSl.$cont);
        
        break;
      }
      case 'int': {
        const compoundSl = createCompoundSlider(param.descr, (value) => {
          options[param.name] = value;
          if($cbAutoUpdate.checked) {
            $bSolve.click();
          }
        }, param.min, param.max, 1, param.default as number);
        $divOptions.append(compoundSl.$cont);
        
        break;
      }
    }
  }
}

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
    $opt.innerText = list.solvers[solver].name;
    $selSolver.append($opt);
  }
})();
$selSolver.addEventListener('change', () => {
  changeSolverOptions($selSolver.value);
});
$bSolve.addEventListener('click', async() => {
  const solver = $selSolver.value;
  const inst = $selInstance.value;

  const name = `${inst} (${solver === '' ? 'Input' : list.solvers[solver].name})`

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
    $profit.innerText = res.profit.toString();
    $log.innerHTML = res.log.replace(/\n/g, '<br>');
  }

  const map = parseTOPMap(content);
  currentMap = map;
  console.log(map);
  const $svg = drawMap(map);
  $mapCanvasContainer.append($svg);

  $fileName.innerText = name;
});

// THUNDER solver

import { ThunderData, FirstThunder, NextThunder } from './thunder.js';

let currentThunder: ThunderData = undefined;

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
