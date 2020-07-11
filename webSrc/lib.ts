
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
