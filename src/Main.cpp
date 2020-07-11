#include "TOP_Data.hpp"

#include "Utils.hpp"

#include <fstream>
#include <filesystem>
#include <random>
#include <chrono>
#include <map>
#include <exception>

#include <httpserver.hpp>
#include <httpserver/http_utils.hpp>

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
namespace chrono = std::chrono;
namespace hs = httpserver;
typedef hs::http::http_utils hs_utils;
using namespace nlohmann;

typedef std::shared_ptr<hs::http_response> http_resp_ptr;

#define PORT 8080
#define INST_PATH "./instances"
#define INST_EXT ".txt"
#define SOL_PATH "./outputs"
#define SOL_EXT ".out"
#define WEB_PATH "./web"

const std::map<std::string, std::string> mimes = {
  {".txt", "text/plain"},
  {".html", "text/html"},
  {".css", "text/css"},
  {".js", "text/javascript"},
  {".json", "application/json"},
  {".png", "image/png"},
};
#define DEF_MIME "text/plain"

#include "Solvers/Solver.hpp"
#include "Solvers/Kevin.hpp"
const std::vector<SolverEntry_s> solvers = {
  { .fn = SolveKevin, .name = "Kevin" },
};
#define DEF_SOLVER 0

template<typename _SEED>
_SEED GenSeed() {
  // Reinstantiate, not that thread safe otherwise...
  std::random_device rd; // Can be not random...
  
  auto millis = std::chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count();
  return (_SEED)rd() ^ (_SEED)millis;
}

class HSR_StaticResources : public hs::http_resource {
  public:
    HSR_StaticResources(std::string basePath, int pathDiscard = 0): http_resource(), basePath(basePath), pathDiscard(pathDiscard) {}
    const http_resp_ptr render_GET(const hs::http_request& req) {
      auto httpPath = req.get_path_pieces();
      if(httpPath.size() < pathDiscard) {
        return http_resp_ptr(new hs::string_response("Path size error", hs_utils::http_not_found));
      }
      // std::string does not meat requirements...
      //auto fullPath = basePath / fs::path::append(httpPath.begin() + pathDiscard, httpPath.end());
      fs::path fullPath = basePath;
      for(auto it = httpPath.begin() + pathDiscard; it != httpPath.end(); ++it) {
        fullPath /= *it;
      }
      //std::cerr << "Path: " << fullPath << std::endl;
      if(fs::exists(fullPath)) {
        // SEGFAULTs if not found?!
        return http_resp_ptr(new hs::file_response(fullPath, hs_utils::http_ok, value_or_default(mimes, fullPath.extension(), DEF_MIME)));
      } else {
        return http_resp_ptr(new hs::string_response("", hs_utils::http_not_found));
      }
    }
  private:
    std::string basePath;
    int pathDiscard;
};

class HSR_List : public hs::http_resource {
  public:
    const http_resp_ptr render_GET(const hs::http_request& req) {
      json j = {
        { "instances", {} },
        { "solvers", {} }
      };
      for(const auto& solver : solvers) {
        j["solvers"].push_back(solver.name);
      }
      for(const auto& file : fs::directory_iterator(INST_PATH)) {
        if(file.path().extension() != INST_EXT) continue;
        j["instances"].push_back(file.path().filename());
      }
      return http_resp_ptr(new hs::string_response(j.dump()));
    }
};

class HSR_Solver : public hs::http_resource {
  public:
    const http_resp_ptr render_GET(const hs::http_request& req) {
      // If absent arg then empty string returned
      int solver = DEF_SOLVER;
      {
        auto solverStr = req.get_arg("solver");
        if(!solverStr.empty()) { // Use default solver if not defined
          try {
            solver = stoi(solverStr);
          } catch(std::invalid_argument) {
            return http_resp_ptr(new hs::string_response("Unable to parse solver", hs_utils::http_not_found));
          }
          if(solver < 0 || solver >= solvers.size()) {
            return http_resp_ptr(new hs::string_response("Unimplemented solver", hs_utils::http_not_found));
          }
        }
      }
      
      auto instStr = req.get_arg("inst");
      auto inst = fs::path(INST_PATH) / instStr;
      if(!fs::exists(inst)) {
        return http_resp_ptr(new hs::string_response("Instance not found", hs_utils::http_not_found));
      }

      // Gen rng
      std::mt19937::result_type seed = GenSeed<std::mt19937::result_type>();
      std::mt19937 rng(seed);

      // Solve
      std::cerr << "LOG: Request to solve \"" << instStr << "\" with solver \"" << solvers[solver].name << "\" and seed " << seed << std::endl;

      TOP_Input in;
      {
        std::ifstream is(inst);
        if(!is) {
          std::cerr << "ERROR: Unable to open instance" << std::endl;
          return http_resp_ptr(new hs::string_response("Unable to open instance", hs_utils::http_internal_server_error));
        }
        is >> in;
        in.name = inst.filename().replace_extension("").string();
      }

      TOP_Output out(in);
      solvers[solver].fn(in, out, rng);

      auto sol = std::to_string(solver) / inst.filename().replace_extension(SOL_EXT);
      {
        auto fullSol = SOL_PATH / sol;
        fs::create_directories(fullSol.parent_path());
        std::ofstream os(fullSol);
        if(!os) {
          std::cerr << "ERROR: Unable to save instance output" << std::endl;
          sol = "";
        }
        os << in << out;
      }

      std::stringstream solutionText;
      solutionText << in << out;
      json j = {
        { "name", in.name },
        { "feasible", out.Feasible() },
        { "profit", out.PointProfit() },
        { "solution", solutionText.str() },
        { "solver", solver },
        { "solverName", solvers[solver].name },
        { "solutionFile", sol.generic_string() } // Relative web path
      };

      std::cerr << "LOG: ";
      if(out.Feasible()) {
        std::cerr << "Solution found, profit " << out.PointProfit();
      } else {
        std::cerr << "Solution not found";
      }
      std::cerr << std::endl;

      return http_resp_ptr(new hs::string_response(j.dump()));
    }
};

template<typename T>
T json_get_or_default(const json& j, T def) {
  return j.is_null() ? def : j.get<T>();
}

class HSR_SolverPlus : public hs::http_resource {
  public:
    const http_resp_ptr render_POST(const hs::http_request& req) {
      // Parse JSON
      json jReq = json::parse(req.get_content());

      // Enforce base scheama
      int solver;
      std::string instStr;
      try {
        solver = json_get_or_default<int>(jReq["solver"], DEF_SOLVER);
        instStr = json_get_or_default<std::string>(jReq["inst"], "");
      } catch(detail::type_error e) {
        return http_resp_ptr(new hs::string_response("Schema error", hs_utils::http_internal_server_error));
      }

      if(instStr.empty()) {
        return http_resp_ptr(new hs::string_response("Missing instance", hs_utils::http_internal_server_error));
      }
      if(solver < 0 || solver >= solvers.size()) {
        return http_resp_ptr(new hs::string_response("Unimplemented solver", hs_utils::http_internal_server_error));
      }
      
      auto inst = fs::path(INST_PATH) / instStr;
      if(!fs::exists(inst)) {
        return http_resp_ptr(new hs::string_response("Instance not found", hs_utils::http_internal_server_error));
      }

      // Gen rng
      std::mt19937::result_type seed = GenSeed<std::mt19937::result_type>();
      std::mt19937 rng(seed);

      // Solve
      std::cerr << "LOG: Request to solve \"" << instStr << "\" with solver \"" << solvers[solver].name << "\" and seed " << seed << std::endl;

      TOP_Input in;
      {
        std::ifstream is(inst);
        if(!is) {
          std::cerr << "ERROR: Unable to open instance" << std::endl;
          return http_resp_ptr(new hs::string_response("Unable to open instance", hs_utils::http_internal_server_error));
        }
        is >> in;
        in.name = inst.filename().replace_extension("").string();
      }

      TOP_Output out(in);
      solvers[solver].fn(in, out, rng, jReq["options"]);

      auto sol = std::to_string(solver) / inst.filename().replace_extension(SOL_EXT);
      {
        auto fullSol = SOL_PATH / sol;
        fs::create_directories(fullSol.parent_path());
        std::ofstream os(fullSol);
        if(!os) {
          std::cerr << "ERROR: Unable to save instance output" << std::endl;
          sol = "";
        }
        os << in << out;
      }

      std::stringstream solutionText;
      solutionText << in << out;
      json j = {
        { "name", in.name },
        { "feasible", out.Feasible() },
        { "profit", out.PointProfit() },
        { "solution", solutionText.str() },
        { "solver", solver },
        { "solverName", solvers[solver].name },
        { "seed", seed },
        { "solutionFile", sol.generic_string() } // Relative web path
      };

      std::cerr << "LOG: ";
      if(out.Feasible()) {
        std::cerr << "Solution found, profit " << out.PointProfit();
      } else {
        std::cerr << "Solution not found";
      }
      std::cerr << std::endl;

      return http_resp_ptr(new hs::string_response(j.dump()));
    }
};

class HSR_Redirect : public hs::http_resource {
  public:
    HSR_Redirect(const std::string& target) : hs::http_resource(), target(target) {}
    const http_resp_ptr render(const hs::http_request& req) {
      auto resp = new hs::http_response(302, "");
      resp->with_header(hs_utils::http_header_location, target);
      return http_resp_ptr(resp);
    }
  private:
    std::string target;
};

int main(int argc, char* argv[]) {

  std::random_device rd; // Can be not random...
  
  auto millis = std::chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count();
  std::mt19937::result_type seed =
    (std::mt19937::result_type)rd() ^
    (std::mt19937::result_type)millis;
  std::mt19937 rng(seed);

  hs::webserver ws = hs::create_webserver(PORT);

  HSR_StaticResources hsrServeWeb(WEB_PATH, 1); // 1 is referred to http path below
  ws.register_resource("/web", &hsrServeWeb, true); // Family, so all nested handled by it

  HSR_Redirect hsrHome("/web/index.html");
  ws.register_resource("/", &hsrHome);
  
  HSR_List hsrList;
  ws.register_resource("/list", &hsrList);

  HSR_StaticResources hsrServeInstances(INST_PATH, 1);
  ws.register_resource("/instances", &hsrServeInstances, true);
  
  HSR_StaticResources hsrServeSolutions(SOL_PATH, 1);
  ws.register_resource("/outputs", &hsrServeSolutions, true);
  
  HSR_Solver hsrSolver;
  ws.register_resource("/solve/{solver}/{inst}", &hsrSolver);
  ws.register_resource("/solve/{inst}", &hsrSolver);

  HSR_SolverPlus hsrSolverPlus;
  ws.register_resource("/solve", &hsrSolverPlus);

  std::cerr << "Starting webserver on port: " << PORT << std::endl;
  if(!ws.start(true)) { // TODO: Allow normal halt, use random gen and make it thread safe... (rng is thread safe? generate seed and send with solution?)
    std::cerr << "ERROR: Unable to start webserver" << std::endl;
  }

  return 0;
}
