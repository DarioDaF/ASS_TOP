
export type ApiList = {
  instances: string[];
  solvers: string[];
}

export type ApiSolve = {
  name: string;
  feasible: boolean;
  profit: number;
  solution: string;
  solver: number;
  solverName: string;
  solutionFile: string;
}

export async function apiGetList() {
  const resp = await fetch('/list');
  return await resp.json() as ApiList;
}

export async function apiSolve(inst: string, solver?: number) {
  const resp = await fetch(solver === undefined ? `/solve/${inst}` : `/solve/${inst}?solver=${solver}`);
  return await resp.json() as ApiSolve;
}

export async function apiInstance(inst: string) {
  const resp = await fetch(`/instances/${inst}`);
  return await resp.text();
}

