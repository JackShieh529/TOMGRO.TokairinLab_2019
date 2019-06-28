#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

namespace tomgro{
template <class T>
using table = std::unordered_map<std::string, T>;

class FileIO{
  private:
    std::vector<std::string> split(std::string& input, char delimiter);
    std::string fixIndex(std::string name, int i);

  public:
    void test();
    void change(table<double>& variables, std::string name, double val);
    void inputData(table<double>& variables, std::string fileName);
    void initializeVariables(table<double>& variables);
    void inputWeather(table<double>& variables, std::string fileName);
};
}
