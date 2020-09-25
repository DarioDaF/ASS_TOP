export type ApiParameter = {
  name: string;
  descr: string;
  type: string;
  default: number|boolean;
  min?: number;
  max?: number;
};

export type ApiSolver = {
  name: string;
  params: ApiParameter[];
};

export type ApiList = {
  instances: string[];
  solvers: ApiSolver[];
}

export type ApiSolve = {
  name: string;
  feasible: boolean;
  profit: number;
  solution: string;
  solver: number;
  solverName: string;
  solutionFile: string;
  log: string;
}

export async function apiGetList() {
  const resp = await fetch('/list');
  return await resp.json() as ApiList;
}

export async function apiSolve(inst: string, solver: number, options: Record<string, unknown> = {}) {
  const resp = await fetch('/solve', {
    method: "POST",
    body: JSON.stringify({ inst, solver, options })
  });
  return await resp.json() as ApiSolve;
}

export async function apiInstance(inst: string) {
  const resp = await fetch(`/instances/${inst}`);
  return await resp.text();
}

