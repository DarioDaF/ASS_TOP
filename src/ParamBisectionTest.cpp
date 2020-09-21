#include <iostream>
#include <fstream>

#include <vector>
#include <map>
#include <set>

#include <thread>
#include <shared_mutex>
#include <ctpl_stl.h>

#include "common/Utils.hpp"
#include "common/TOP_Data.hpp"
#include "greedy/TOP_Greedy.hpp"

using namespace std;

#define N_WT 50
#define N_MDEV 50

#define WT_MIN 0.01
#define WT_MAX 1.5

#define MDEV_MIN 0.0
#define MDEV_MAX 4.0

/*

Se parto da una mesh poco fitta e cerco quanti spazi devo approfondire
posso trovare quali sono vedendo quelli con profitti diversi in un intervallo
lungo uno dei due assi qualsiasi (quadrato) e puntare in mezzo: passo di
raffinamento lungo un asse solo e in una colonna sola.

Meglio raffinare lungo C o B prima? prima quello che taglia di più che con 1 test
solo sembra essere MDEV più fitto, quindi taglia lungo wTime prima.

In alternativa parti con una matrice vuota e propaga i livelli?
Riempi in modo forzato lungo gli assi se X ...-1... X avviene
prima lungo un asse e poi l'altro, dovrebbe fillare i quadrati!


Dato un quadrato del tipo:

X  .  X

.  O  .

X  .  X

Con X già trovati E NON TUTTI E 4 UGUALI allora provo O.
Se O appare uguale a uno delle X marco i . ai lati in comune
con il valore di O, tutti i punti che non sono fillati vengono calcolati
e poi il processo continua su ciasucno dei quadrati nuovi non autofillati
da questa procedura.

Il processo si interrompe quando la distanza fra le X e le O è inferiore al
valore prefissato, è importante che il rapporto fra gli assi sia lo stesso
fra la griglia più fissa e quella più grezza poiché viene sempre zoommato
con lo stesso ratio (1/2 progressivo).

Posso usare la pool per determinare il livello di profondità e auto aggiungere gli
elementi nuovi da calcolare di passo in passo (attenzione, i 2 quadrati si toccano per
punti in comune!, quindi alcuni . potrebbero essere già presenti a priori).
Oppure aggiungo un check se già presente?
Il risultato dovrebbe essere sempre uguale però quindi è ok.
Occhio alla lettura però, usare atomic<>?

Per lo store è importante se usato un map avere int come chiavi, evita
float/double comparison, quindi scala già di base sulla risoluzione massima.

Posso già accodare i valori delle X che conosco nel singolo thread in modo da
evitare il più possibile accessi al map.

Quindi un thread dovrebbe dato un quadrato di quel tipo:
- Calcolare O
- Determinare se può inferire i valori di . e assegnarli al map tenendone una copia locale
- Calcolare i . che non può inferire (dopo aver controllato che non sono nel map, dovrebbe controllare anche che nessuno li sta valutando?, nah è veloce)
(siccome è veloce non è che è sempre meglio ricalcolare i .? tanto il miglioramento avviene sulla risoluzione/grana?)
- Se non ha raggiunto le condizioni di stop accodare da 0 a 4 funzioni extra per i 4 quadranti

Il main dunque calcola le X degli estremi del quadrato, poi passa ai threaed il primo pool,
infine attente stop(true) visto che il pool viene aggiornato prima di morire il thread

La risoluzione è fissata quindi si può determinare la profondità, a quel punto le divisioni diventano (2^depth + 1)

Per essere precisi questo metodo non conta la possibilità di avere zone come:
X Y X negli intervalli, quindi sarebbe meglio non partire da una griglia estremamente grezza

In alterantiva si può porre che fino a una certa depth viene sempre eseguito il calcolo senza controlli

*/

typedef int sq_vals_t[2][2]; // Profitto degli angoli (check semplice, potrebbe essere accompagnato da checksum soluzione, non assicura confronto corretto)
// A B     A
//     -> B C
// C D     D
struct pos_t {
  int wTimeIdx;
  int maxDevIdx;
};
bool operator<(const pos_t& a, const pos_t& b) {
  return a.wTimeIdx < b.wTimeIdx || (a.wTimeIdx == b.wTimeIdx &&
    a.maxDevIdx < b.maxDevIdx
  );
}

#define MIN_DEPTH 5
#define MAX_DEPTH 8

// Se depth 1 (primo thread, depth 0 calcola gli angoli) allora gli sq hanno indici sono in 0 e 2^MAX_DEPTH e pos risulta 2^(MAX_DEPTH-1)
// Se N allora partendo da 0 ottengo 2^(MAX_DEPTH-N+1) come spigoli e pos di 2^(MAX_DEPTH-N)
// Quindi il raggio è tale da avere al passo successivo POS +- 2^(MAX_DEPTH-N-1)
// Ovviamente 2^X = 1 << X
// Fissa il seed per avere una soluzione stabile a blocchi

double wTimeFromIdx(int wTimeIdx) {
  return WT_MIN + wTimeIdx * (WT_MAX - WT_MIN) / (1 << MAX_DEPTH);
}

double maxDevFromIdx(int maxDevIdx) {
  return MDEV_MIN + maxDevIdx * (MDEV_MAX - MDEV_MIN) / (1 << MAX_DEPTH);
}

void runThread(int id, ctpl::thread_pool& pool, const TOP_Input& in, mt19937::result_type seed, safe_map<pos_t, int>& profits, sq_vals_t sq_vals, pos_t pos, int depth) {
  // Calcola in O
  int oProfit = -1;
  {
    mt19937 rng(seed);
    TOP_Output out(in);
    GreedySolver(in, out, rng, 1.0, wTimeFromIdx(pos.wTimeIdx), maxDevFromIdx(pos.maxDevIdx), 0);
    oProfit = out.PointProfit();
  }

  // Agguingi al map
  profits.insert(pos, oProfit);

  sq_vals_t dots = { {-1, -1}, {-1, -1} };

  // Inferisci i ., prendili dal map o calcolali

  pos_t top { .wTimeIdx = pos.wTimeIdx, .maxDevIdx = pos.maxDevIdx - (1 << (MAX_DEPTH - depth)) };
  pos_t left { .wTimeIdx = pos.wTimeIdx - (1 << (MAX_DEPTH - depth)), .maxDevIdx = pos.maxDevIdx };
  pos_t right { .wTimeIdx = pos.wTimeIdx + (1 << (MAX_DEPTH - depth)), .maxDevIdx = pos.maxDevIdx };
  pos_t bottom { .wTimeIdx = pos.wTimeIdx, .maxDevIdx = pos.maxDevIdx + (1 << (MAX_DEPTH - depth)) };

  if(sq_vals[0][0] == oProfit || sq_vals[0][1] == oProfit) {
    dots[0][0] = oProfit;
    profits.insert(top, dots[0][0]);
  } else if(!profits.find(top, dots[0][0])) {
    {
      mt19937 rng(seed);
      TOP_Output out(in);
      GreedySolver(in, out, rng, 1.0, wTimeFromIdx(top.wTimeIdx), maxDevFromIdx(top.maxDevIdx), 0);
      dots[0][0] = out.PointProfit();
    }
    profits.insert(top, dots[0][0]);
  }
  if(sq_vals[0][0] == oProfit || sq_vals[1][0] == oProfit) {
    dots[0][1] = oProfit;
    profits.insert(left, dots[0][1]);
  } else if(!profits.find(left, dots[0][1])) {
    {
      mt19937 rng(seed);
      TOP_Output out(in);
      GreedySolver(in, out, rng, 1.0, wTimeFromIdx(left.wTimeIdx), maxDevFromIdx(left.maxDevIdx), 0);
      dots[0][1] = out.PointProfit();
    }
    profits.insert(left, dots[0][1]);
  }
  if(sq_vals[0][1] == oProfit || sq_vals[1][1] == oProfit) {
    dots[1][0] = oProfit;
    profits.insert(right, dots[1][0]);
  } else if(!profits.find(right, dots[1][0])) {
    {
      mt19937 rng(seed);
      TOP_Output out(in);
      GreedySolver(in, out, rng, 1.0, wTimeFromIdx(right.wTimeIdx), maxDevFromIdx(right.maxDevIdx), 0);
      dots[1][0] = out.PointProfit();
    }
    profits.insert(right, dots[1][0]);
  }
  if(sq_vals[1][0] == oProfit || sq_vals[1][1] == oProfit) {
    dots[1][1] = oProfit;
    profits.insert(bottom, dots[1][1]);
  } else if(!profits.find(bottom, dots[1][1])) {
    {
      mt19937 rng(seed);
      TOP_Output out(in);
      GreedySolver(in, out, rng, 1.0, wTimeFromIdx(bottom.wTimeIdx), maxDevFromIdx(bottom.maxDevIdx), 0);
      dots[1][1] = out.PointProfit();
    }
    profits.insert(bottom, dots[1][1]);
  }

  // Ora vedi quali quadranti calcolare in modo più fitto
  if(depth >= MAX_DEPTH) {
    return;
  }

  if(depth < MIN_DEPTH || sq_vals[0][0] != oProfit) {
    pool.push(runThread, ref(pool), ref(in), seed, ref(profits),
      sq_vals_t { { sq_vals[0][0], dots[0][0] }, { dots[0][1], oProfit } },
      pos_t { .wTimeIdx = pos.wTimeIdx - (1 << (MAX_DEPTH - depth - 1)), .maxDevIdx = pos.maxDevIdx - (1 << (MAX_DEPTH - depth - 1)) },
      depth + 1
    );
  }
  if(depth < MIN_DEPTH || sq_vals[0][1] != oProfit) {
    pool.push(runThread, ref(pool), ref(in), seed, ref(profits),
      sq_vals_t { { dots[0][0], sq_vals[0][1] }, { oProfit, dots[1][0] } },
      pos_t { .wTimeIdx = pos.wTimeIdx + (1 << (MAX_DEPTH - depth - 1)), .maxDevIdx = pos.maxDevIdx - (1 << (MAX_DEPTH - depth - 1)) },
      depth + 1
    );
  }
  if(depth < MIN_DEPTH || sq_vals[1][0] != oProfit) {
    pool.push(runThread, ref(pool), ref(in), seed, ref(profits),
      sq_vals_t { { dots[0][1], oProfit }, { sq_vals[1][0], dots[1][1] } },
      pos_t { .wTimeIdx = pos.wTimeIdx - (1 << (MAX_DEPTH - depth - 1)), .maxDevIdx = pos.maxDevIdx + (1 << (MAX_DEPTH - depth - 1)) },
      depth + 1
    );
  }
  if(depth < MIN_DEPTH || sq_vals[1][1] != oProfit) {
    pool.push(runThread, ref(pool), ref(in), seed, ref(profits),
      sq_vals_t { { oProfit, dots[1][0] }, { dots[1][1], sq_vals[1][1] } },
      pos_t { .wTimeIdx = pos.wTimeIdx + (1 << (MAX_DEPTH - depth - 1)), .maxDevIdx = pos.maxDevIdx + (1 << (MAX_DEPTH - depth - 1)) },
      depth + 1
    );
  }

}

int main() {
  auto nHWThreads = thread::hardware_concurrency();

  cout << "HW Threads: " << nHWThreads << endl;
  
  cout << "Initializing thread pool" << endl;
  ctpl::thread_pool pool(nHWThreads);

  cout << "Reading input" << endl;
  TOP_Input in;
  {
    //ifstream ifs("instances/p5.4.q.txt");
    //ifstream ifs("instances/p7.4.t.txt");
    //ifstream ifs("instances/p4.3.d.txt");
    //ifstream ifs("instances/p5.2.f.txt");
    ifstream ifs("instances/p5.4.z.txt");
    if(!ifs) {
      throw new runtime_error("Unable to open file");
    }
    ifs >> in;
  }

  random_device rd; // Can be not random...
  auto millis = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count();
  mt19937::result_type seed =
    (mt19937::result_type)rd() ^
    (mt19937::result_type)millis;

  safe_map<pos_t, int> profits;

  sq_vals_t sq_vals = { {-1, -1}, {-1, -1} };

  // Calcola bordi
  cout << "Computing corner values" << endl;
  {
    mt19937 rng(seed);
    TOP_Output out(in);
    GreedySolver(in, out, rng, 1.0, WT_MIN, MDEV_MIN, 0);
    sq_vals[0][0] = out.PointProfit();
  }
  {
    mt19937 rng(seed);
    TOP_Output out(in);
    GreedySolver(in, out, rng, 1.0, WT_MIN, MDEV_MAX, 0);
    sq_vals[0][1] = out.PointProfit();
  }
  {
    mt19937 rng(seed);
    TOP_Output out(in);
    GreedySolver(in, out, rng, 1.0, WT_MAX, MDEV_MIN, 0);
    sq_vals[1][0] = out.PointProfit();
  }
  {
    mt19937 rng(seed);
    TOP_Output out(in);
    GreedySolver(in, out, rng, 1.0, WT_MAX, MDEV_MAX, 0);
    sq_vals[1][1] = out.PointProfit();
  }

  cout << "Starting pool" << endl;
  pool.push(runThread, ref(pool), ref(in), seed, ref(profits), sq_vals,
    pos_t { .wTimeIdx = 1 << (MAX_DEPTH-1), .maxDevIdx = 1 << (MAX_DEPTH-1) },
    1
  );

  cout << "Waiting on pool" << endl;
  //this_thread::sleep_for(chrono::seconds(1));
  do {
    this_thread::sleep_for(chrono::seconds(1));
    cerr << ".";
  } while(pool.n_idle() < pool.size());
  cerr << endl;
  pool.stop(true); // Wait for solution

  cout << "Computations: " << profits._map.size() << endl;

  set<int> profits_distinct;
  for(const auto& mapEntry : profits._map) {
    profits_distinct.insert(mapEntry.second);
  }

  cout << "Distincts: " << profits_distinct.size() << endl;

  cout << "Best: " << *profits_distinct.rbegin() << " | Worst: " << *profits_distinct.begin() << endl;

  {
    ofstream ofs("outputs/cross.tsv");
    if(!ofs) {
      throw new runtime_error("Unable to open file");
    }
    for(const auto& mapEntry : profits._map) {
      ofs << wTimeFromIdx(mapEntry.first.wTimeIdx) << '\t' << maxDevFromIdx(mapEntry.first.maxDevIdx) << '\t' << mapEntry.second << endl;
    }
  }

  return 0;
}
