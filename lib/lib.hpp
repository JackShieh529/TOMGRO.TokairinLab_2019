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
template <class T>
using table = std::unordered_map<std::string, T>;

class FileIO;
class Calc;

class FileIO{
  private:
    Calc* calc;
    std::vector<std::string> split(std::string& input, char delimiter);

  public:
    FileIO();
    void test();
    std::string fixIndex(std::string name, int i);
    void change(table<double>& variables, std::string name, double val);
    void inputData(table<double>& variables, std::string fileName);
    void initializeVariables(table<double>& variables);
    void inputWeather(table<double>& variables, std::string fileName);
};

class Calc{
  private:
    FileIO* fileio;
  public:
  Calc();
    Calc(FileIO* pfileio);
    void sunrise(table<double>& var);
    double tabex(table<double>& var, std::string val, std::string arg, double dummy, int k);
    void calcWeather(table<double>& var);
    void ghouse(table<double>& var);
    void devfast(table<double>& var);
    void photo(table<double>& var);
    void resp(table<double>& var);
    void dmrate(table<double>& var);
    void devrate(table<double>& var);
    void losrate(table<double>& var);
    void intgrat(table<double>& var);
};
}
