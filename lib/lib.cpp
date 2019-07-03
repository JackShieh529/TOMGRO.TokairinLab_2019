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
  /**********/

  FileIO::FileIO(){
    calc = new Calc(this);
  }

  std::string FileIO::fixIndex(std::string name, int i){
    return name + "(" + std::to_string(i) + ")";
  }

  void FileIO::change(table<double>& variables, std::string name, double val){
    auto itr = variables.find(name);
    if(itr != variables.end()){
      variables[name] = val;
      //std::cout << "CHANGE:" << name << ":" << val << std::endl;
    }else{
      variables.insert(param{name, val});
      //std::cout << "ADD:" << name << ":" << val << std::endl;
    }
  }

  void FileIO::test(){
    std::cout << "Hello World!" << std::endl;
  }

  void FileIO::inputData(table<double>& variables, std::string fileName){
    //C-14 Get parameters from fileName and set
    std::ifstream ifs(fileName);
    std::vector<std::string> pair(2);
    if(!ifs.is_open()){
      change(variables, "Error", -1); //fault to open
      return;
    }
    
    while (!ifs.eof()){
      try{
        std::string buffer;
        ifs >> buffer;
        if(buffer == "" || buffer == "\n" || buffer == "\t" ) continue;
        pair = split(buffer, ',');
        change(variables, pair[0], std::stod(pair[1]));
      }catch(std::exception e_range){
        continue;
      }
    }
  }

  void FileIO::initializeVariables(table<double>& variables){
    //C-13 Set initial values
    for(int i=0;i<variables["NL"];i++){
      change(variables, fixIndex("LVSN",i), 0);
      change(variables, fixIndex("STMS",i), 0);
      change(variables, fixIndex("WLVS",i), 0);
      change(variables, fixIndex("WSTM",i), 0);
      change(variables, fixIndex("LFAR",i), 0);
    }

    for(int i=0;i<variables["NF"];i++){
      change(variables, fixIndex("FRTN",i), 0);
      change(variables, fixIndex("WFRT",i), 0);
    }

    change(variables, "PLSTN", variables["PLSTNI"]);
    change(variables, "CPOOL", 0.0);
    change(variables, "LVSN(0)", variables["LVSNI"]*variables["PLM2"]);
    change(variables, "BTOTNLV", variables["LVSNi"]*variables["plm2"]);
    change(variables, "STMS(0)", variables["LVSN(0)"]);
    change(variables, "WLVS(0)", variables["WLVSI"]*variables["PLM2"]);
    change(variables, "WSTM(0)", variables["WLVS(0)"]*variables["FRSTEM(0)"]);
    change(variables, "LFAR(0)", variables["LFARI"]*variables["PLM2"]);
    change(variables, "XLAI", variables["LFAR(0)"]);
    change(variables, "TOTWML", 0.0);
    change(variables, "ATL", 0.0);
    change(variables, "ATV", 0.0);
    change(variables, "TOTWST", 0.0);
    change(variables, "WTOTF", 0.0);
    change(variables, "ASTOTL", variables["XLAI"]);
    change(variables, "WSTOTL", 0.0);
    change(variables, "FWFR10", 0.0);
    change(variables, "APFFW", 0.0);
    change(variables, "ATT", 0.0);
  }

  void FileIO::inputWeather(table<double>& variables, std::string fileName){
    //C-20 Get weather parameters from fileName and set
    //要修正
    std::ifstream ifs(fileName);
    std::vector<std::string> pair;
    std::string header[7] = {"year", "jd", "rad", "max_temp", "min_temp", "rain", "par"};

    if(!ifs.is_open()){
      change(variables, "Error", -1); //fault to open
      return;
    }
    
    //First data
    change(variables, "XLAT", 31.2);
    change(variables, "XLONG", 34.4);
    change(variables, "PARFAC", 12.07);
    change(variables, "PARDAT", 0.5); //?

    int row = -2;
    while (!ifs.eof()){
      try{
        std::string buffer;
        ifs >> buffer;
        row++;
        if(row < 0) continue;
        if(buffer == "" || buffer == "\n" || buffer == "\t" ) continue;
        pair = split(buffer, ',');
        //For waeather file
        for(int col=0;col<pair.size();col++){
          change(variables, fixIndex(header[col], row), std::stod(pair[col]));
        }
      }catch(std::exception e_range){
        continue;
      }
    }
    change(variables, "XLANG", variables["SOLRAD"]*23.923);
    calc->sunrise(variables);
    if(variables["PARDAT"] < 0) variables["PARO"] = variables["XLANG"] / variables["PARFAC"];
  }

  Calc::Calc(){}

  Calc::Calc(FileIO* pfileio){
    fileio = pfileio;
  }

  void Calc::sunrise(table<double>& val){
    val["HEMIS"] = val["XLAT"]/std::abs(val["XLAT"]);
    val["DEC"] = -1*val["HEMIS"]*23.4*std::cos(2*M_PI*(val["IJUL"]+10)/365);
    val["SNDC"] = std::sin(val["RAD"]*val["DEC"]);
    val["CSDC"] = std::cos(val["RAD"]*val["DEC"]);
    val["SNLT"] = std::sin(val["RAD"]*val["XLAT"]);
    val["CSLT"] = std::cos(val["RAD"]*val["XLAT"]);
    val["SSIN"] = val["SNDC"]*val["SNLT"];
    val["CCOS"] = val["CSDC"]*val["CSLT"];
    val["TT"] = val["SSIN"]/val["CCOS"];
    val["AS"] = std::asin(val["TT"]);
    val["DAYL"] = 12*(M_PI+2*val["AS"])/M_PI;
    if(val["XLAY"] < 0) val["DAYL"] = 24 - val["DAYL"];
    val["XSNUP"] = 12 - val["DAYL"] / 2;
    val["XSNDN"] = 12 + val["DAYL"] / 2;
  }

  void Calc::calcWeather(table<double>& val){
    //C-30 WCALC
    val["SDNT"] = val["SDN"];
    val["SUPT"] = val["SUP"];
    val["TMINT"] = val["TMIN"];
    val["TMAXT"] = val["TMAX"];

    if(val["JDAY"] <= 1){
      val["SDNY"] = val["SDN"];
      val["SUPY"] = val["SUP"];
      val["TMINY"] = val["TMIN"];
      val["TMAXY"] = val["TMAX"];
    }

    for(int i=0;i<25;i++){
      if(i < val["SUP"]+2.0){
        val["TAU"] = 3.1417 * (val["SDNY"] - val["SUPY"] - 2)/(val["SDNY"] - val["SUPY"]);
        val["TLIN"] = val["TMINY"] + ((val["TMAXY"] - val["TMINY"]) * std::sin(val["TAU"]));
        val["HDARK"] = 24 - val["SDNY"] + val["SUP"] + 2;
        val["SLOPE"] = (val["TLIN"] - val["TMIN"]) / val["HDARK"];
        val[fileio->fixIndex("THR", i)] = val["TLIN"] - val["SLOPE"] * (i + 24 - val["SDNY"]);
        break;
      }else if(i > val["SDN"]){
        val["TAU"] = 3.1417 * (val["SDNY"] - val["SUP"] - 2)/(val["SDN"] - val["SUP"]);
        val["TLIN"] = val["TMIN"] + ((val["TMAX"] - val["TMIN"]) * std::sin(val["TAU"]));
        val["HDARK"] = 24 - val["SDN"] + val["SUPT"] + 2;
        val["SLOPE"] = (val["TLIN"] - val["TMINT"]) / val["HDARK"];
        val[fileio->fixIndex("THR", i)] = val["TLIN"] - val["SLOPE"] * (i + 24 - val["SDN"]);
        break;
      }
    }
    val["SDNY"] = val["SDN"];
    val["SUPY"] = val["SUP"];
    val["TMINY"] = val["TMIN"];
    val["TMAXY"] = val["TMAX"];
    val["DL"] = val["SDN"] - val["SUP"];
    
    for(int i=0;i<25;i++){
      val[fileio->fixIndex("RAD", i)] = 0;
      if(i < val["SUP"] || i > val["SUP"]) return;
      val[fileio->fixIndex("RAD", i)] = 3.1417 / (2 * val["DL"]) * val["SOLRAD"] * std::sin(3.1417 * (i - val["SUP"]) / val["DL"]);
    }
  }
}