
export type Color = {
  r: number;
  g: number;
  b: number;
}

export function lerp(x: number, c1: Color, c2: Color) {
  const invX = 1 - x;
  return {
    r: c1.r * invX + c2.r * x,
    g: c1.g * invX + c2.g * x,
    b: c1.b * invX + c2.b * x,
  }
}

export function toHex(n: number, padding: number = 2) {
  let x = n.toString(16);
  while(x.length < padding) {
    x = '0' + x;
  }
  return x.toUpperCase();
}

export function toName(c: Color) {
  return '#' + toHex(Math.round(c.r * 255)) + toHex(Math.round(c.g * 255)) + toHex(Math.round(c.b * 255));
}

export function emptyElement($el: Element) {
  while($el.lastChild) {
    $el.removeChild($el.lastChild);
  }
}

export function createSvg(type: string, attrs: Record<string, any>) {
  const $el = document.createElementNS('http://www.w3.org/2000/svg', type);
  for(const [name, value] of Object.entries(attrs)) {
    $el.setAttribute(name, value);
  }
  return $el;
}

export function createSvgArrow(type: string, attrs: Record<string, any>, $svg: SVGElement) {
  // getElementById is not valid for svg
  let idHead = 'arrowHead';
  if(attrs.stroke) {
    // Should esape stroke correctly...
    idHead += '-' + (attrs.stroke as string).replace('#', '_');
  }
  if(!$svg.querySelector('#' + idHead)) {
    const $marker = createSvg('marker', {
      id: idHead, markerWidth: 3, markerHeight: 3,
      refX: 3, refY: 1.5, orient: 'auto'
    });
    $marker.append(createSvg('polygon', {
      points: `0 0, 0 3, 3 1.5`,
      fill: attrs.stroke
    }));
    let $defs = $svg.querySelector('defs');
    if(!$defs) {
      $defs = createSvg('defs', {}) as SVGDefsElement;
      $svg.append($defs);
    }
    $defs.append($marker);
  }
  return createSvg(type, { ...attrs, 'marker-end': 'url(#' + idHead + ')' });
}

export function createCompoundSlider(label: string, onChange: (value: number) => void, min: number, max: number, step: number, def: number) {
  const $label = document.createElement('label');
  $label.innerText = label;
  const $range = document.createElement('input');
  $range.type = 'range';
  $range.min = min.toString();
  $range.max = max.toString();
  $range.step = step.toString();
  $range.value = def.toString();
  //$range.id = id + '__range';
  const $number = document.createElement('input');
  $number.type = 'number';
  $number.min = min.toString();
  $number.max = max.toString();
  $number.step = step.toString();
  $number.value = def.toString();
  //$numeric.id = id;

  const $div = document.createElement('div');
  $div.append($label);
  $div.append($range);
  $div.append($number);

  $range.addEventListener('change', (e) => {
    $number.value = $range.value;
    onChange(parseFloat($number.value));
  });
  $number.addEventListener('change', (e) => {
    $range.value = $number.value;
    onChange(parseFloat($number.value));
  });

  //onChange(def);

  return { $cont: $div, $input: $number };
}

export type Point = {
  x: number;
  y: number;
}

export function pDist(p1: Point, p2: Point) {
  return Math.sqrt((p1.x - p2.x) ** 2 + (p1.y - p2.y) ** 2);
}
export function pDistSq(p1: Point, p2: Point) {
  return (p1.x - p2.x) ** 2 + (p1.y - p2.y) ** 2;
}
export function pLen(p: Point) {
  return Math.sqrt(p.x ** 2 + p.y ** 2);
}
export function pLenSq(p: Point) {
  return p.x ** 2 + p.y ** 2;
}
export function pAdd(p1: Point, p2: Point) {
  return { x: p1.x + p2.x, y: p1.y + p2.y };
}
export function pSub(p1: Point, p2: Point) {
  return { x: p1.x - p2.x, y: p1.y - p2.y };
}
export function pMul(p: Point, m: number) {
  return { x: p.x * m, y: p.y * m };
}
export function pNorm(p: Point) {
  return pMul(p, 1/pLen(p));
}
