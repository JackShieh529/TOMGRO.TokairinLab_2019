#pragma once
#define _USE_MATH_DEFINES

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <cmath>

namespace tomgro{
struct DefaultNum{
  double value = 1e10;
};
using table = std::unordered_map<std::string, DefaultNum>;

class FileIO;
class Calc;

class FileIO{
  private:
    Calc* calc;

  public:
    FileIO();
    void test();
    std::vector<std::string> split(std::string& input, char delimiter);
    std::string fixIndex(std::string name, int i);
    void change(table& variables, std::string name, double val);
    void inputData(table& variables, std::string fileName);
    void initializeVariables(table& variables);
    void inputWeather(table& variables, std::string fileName);
    void output(table& variables);
};

class Calc{
  private:
    FileIO* fileio;
  public:
  Calc();
    Calc(FileIO* pfileio);
    void sunrise(table& var, std::string IJUL, std::string XSNUP, std::string XSNDN, std::string XLAT, std::string XLONG);
    double tabex(table& var, std::string val, std::string arg, double dummy, int k);
    void calcWeather(table& var);
    void ghouse(table& var);
    void devfast(table& var);
    void photo(table& var);
    void resp(table& var);
    void dmrate(table& var);
    void devrate(table& var);
    void losrate(table& var);
    void intgrat(table& var);
};
}
