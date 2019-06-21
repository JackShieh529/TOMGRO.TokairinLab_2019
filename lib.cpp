#include "lib.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>

namespace tomgro{
  void tomgro::FileIO::test(){
    std::cout << "Hello World!" << std::endl;
  }
  std::unordered_map<std::string, double> tomgro::FileIO::inputData(std::string fileName){}
  std::unordered_map<std::string, double> tomgro::FileIO::initialCondition(std::string fileName){}
  std::unordered_map<std::string, double> tomgro::FileIO::inputWeather(std::string fileName){}
}