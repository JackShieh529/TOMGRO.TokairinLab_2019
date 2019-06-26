#include "lib.hpp"

int main(){
  tomgro::FileIO io;
  tomgro::table<double> data({});
  io.inputData(data, "../src/MGT.csv");
  io.inputData(data, "../src/CropParam.csv");
  io.initializeVariables(data);
  std::cout << data["LVSN(2)"] << std::endl;
  return 0;
}