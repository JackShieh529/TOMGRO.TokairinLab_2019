#include "lib.hpp"
#include <map>

int main(){
  tomgro::FileIO io;
  tomgro::table<double> data;
  io.inputData(data, "../src/MGT.csv");
  io.inputData(data, "../src/CropParam.csv");
  io.initializeVariables(data);
  std::map<std::string, double> newmap(data.begin(), data.end());
  for(auto p:newmap){
    std::cout << p.first << ":" << p.second << std::endl;
  }
  return 0;
}