#include "lib.hpp"
#include <iostream>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>

namespace tomgro{
  //PRIVATE FUNCTION

  private std::vector<std::string> split(std::string& input, char delimiter){
    std::istringstream stream(input);
    std::string field;
    std::vector<std::string> result;
    while (getline(stream, field, delimiter)) {
        result.push_back(field);
    }
    return result;
  }

  private fixIndex(std::string &replacedStr){

  }

  void FileIO::test(){
    std::cout << "Hello World!" << std::endl;
  }

  std::unordered_map<std::string, double> FileIO::inputData(std::string fileName){
    std::ifstream ifs(fileName);
    if(!ifs.is_open()){
      std::unordered_map<std::string, double> fault{{"Error", -1}};
      return fault;
    }
    while (!ifs.eof()){
      std::string buffer;
      ifs >> buffer;
      std::cout << buffer << std::endl;
    }
  }
  //std::unordered_map<std::string, double> FileIO::initialCondition(std::string fileName);
  //std::unordered_map<std::string, double> FileIO::inputWeather(std::string fileName);
}