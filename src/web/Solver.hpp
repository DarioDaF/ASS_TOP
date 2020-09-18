#ifndef WEB_SOLVER_HPP
#define WEB_SOLVER_HPP

#include <string>
#include <random>
#include <nlohmann/json.hpp>
#include "../common/TOP_Data.hpp"

namespace Web {

  template<typename T>
  struct TypeInfo {
    static std::string name() { return typeid(T).name(); }
  };

  template<>
  struct TypeInfo<float> {
    static std::string name() { return "float"; }
  };
  template<>
  struct TypeInfo<double> {
    static std::string name() { return "float"; }
  };
  template<>
  struct TypeInfo<bool> {
    static std::string name() { return "bool"; }
  };
  template<>
  struct TypeInfo<long int> {
    static std::string name() { return "int"; }
  };
  template<>
  struct TypeInfo<int> {
    static std::string name() { return "int"; }
  };
  template<>
  struct TypeInfo<unsigned int> {
    static std::string name() { return "int"; }
  };
  template<>
  struct TypeInfo<unsigned long int> {
    static std::string name() { return "int"; }
  };

  class Parameter {
    public:
      Parameter(std::string name, std::string descr) : name(name), descr(descr) {};
      std::string name;
      std::string descr;
      //virtual void fromJson(nlohmann::json j) = 0;
      virtual nlohmann::json toJsonDescr() {
        return nlohmann::json { { "name", name }, { "descr", descr } };
      };
  };
  template<typename T>
  class TParameter : public Parameter {
    public:
      TParameter(std::string name, std::string descr, T def) : Parameter(name, descr), def(def) {};
      T def;
      T get(nlohmann::json j) {
        if(j.contains(name)) {
          return j[name];
        }
        return def;
      }
      /*
      void fromJson(nlohmann::json j) override {
        value = j;
      }
      */
      virtual nlohmann::json toJsonDescr() override {
        auto j = Parameter::toJsonDescr();
        j["type"] = TypeInfo<T>::name();
        j["default"] = def;
        return j;
      };
  };
  template<typename T>
  class RParameter : public TParameter<T> {
    public:
      RParameter(std::string name, std::string descr, T def, T min, T max) : TParameter<T>(name, descr, def), min(min), max(max) {};
      T min, max;
      virtual nlohmann::json toJsonDescr() override {
        auto j = TParameter<T>::toJsonDescr();
        j["min"] = min;
        j["max"] = max;
        return j;
      };
  };

};

class AbstractWebSolver {
  protected:
    virtual std::vector<Web::Parameter*> GetParameters() {
      return {};
    }
  public:
    virtual std::string name() = 0;

/*    // WARNING: LoadParameters makes default value not valid and problems if multiple solving at the same time
    void LoadParameters(const nlohmann::json& j) {
      for(const auto param : GetParameters()) {
        if(j.contains(param->name)) {
          param->fromJson(j[param->name]);
        }
      }
    }
*/

    nlohmann::json GetParametersDescription() {
      auto j = nlohmann::json::array();
      for(const auto param : GetParameters()) {
        j.push_back(param->toJsonDescr());
      }
      return j;
    }

    virtual void Solve(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options, std::ostream& log) = 0;
};

/*
typedef void (*SolveFunction)(const TOP_Input &in, TOP_Output& out, std::mt19937& rng, nlohmann::json& options);

struct SolverEntry_s {
  SolveFunction fn;
  std::string name;
};
*/

#endif
