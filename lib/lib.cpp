#include "lib.hpp"

namespace tomgro{
  using param = std::pair<std::string, double>;

  ///PRIVATE FUNCTION
  /**********/
  std::vector<std::string> FileIO::split(std::string& input, char delimiter){
    std::istringstream stream(input);
    std::string field;
    std::vector<std::string> result;
    while (getline(stream, field, delimiter)) {
        result.push_back(field);
    }
    return result;
  }

  std::string FileIO::fixIndex(std::string name, int i){
    return name + "(" + std::to_string(i) + ")";
  }
  /**********/

  void FileIO::test(){
    std::cout << "Hello World!" << std::endl;
  }

  void FileIO::inputData(table<double>& variables, std::string fileName){
    //C-14 Get parameters from fileName and set
    std::ifstream ifs(fileName);
    std::vector<std::string> pair(2);
    if(!ifs.is_open()){
      variables.insert(param{"Error", -1}); //fault to open
      return;
    }
    
    while (!ifs.eof()){
      try{
        std::string buffer;
        ifs >> buffer;
        if(buffer == "") continue;
        pair = split(buffer, ',');
        variables.insert(param{pair[0], std::stod(pair[1])});
      }catch(std::exception e_range){
        continue;
      }
    }
  }

  void FileIO::initializeVariables(table<double>& variables){
    //C-13 Set initial values
    for(int i=0;i<variables["NL"];i++){
      variables.insert(param{fixIndex("LVSN",i), 0});
      variables.insert(param{fixIndex("STMS",i), 0});
      variables.insert(param{fixIndex("WLVS",i), 0});
      variables.insert(param{fixIndex("WSTM",i), 0});
      variables.insert(param{fixIndex("LFAR",i), 0});
    }

    for(int i=0;i<variables["NF"];i++){
      variables.insert(param{fixIndex("FRTN",i), 0});
      variables.insert(param{fixIndex("WFRT",i), 0});
    }
  }
  //std::unordered_map<std::string, double> FileIO::inputWeather(std::string fileName);
}