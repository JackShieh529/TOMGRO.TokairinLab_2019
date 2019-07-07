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

  void FileIO::change(table<double>& var, std::string name, double val){
    auto itr = var.find(name);
    if(itr != var.end()){
      var[name] = val;
      //std::cout << "CHANGE:" << name << ":" << val << std::endl;
    }else{
      var.insert(param{name, val});
      //std::cout << "ADD:" << name << ":" << val << std::endl;
    }
  }

  void FileIO::test(){
    std::cout << "Hello World!" << std::endl;
  }

  void FileIO::inputData(table<double>& var, std::string fileName){
    //C-14 Get parameters from fileName and set
    std::ifstream ifs(fileName);
    std::vector<std::string> pair(2);
    if(!ifs.is_open()){
      change(var, "Error", -1); //fault to open
      return;
    }
    
    while (!ifs.eof()){
      try{
        std::string buffer;
        ifs >> buffer;
        if(buffer == "" || buffer == "\n" || buffer == "\t" ) continue;
        pair = split(buffer, ',');
        change(var, pair[0], std::stod(pair[1]));
      }catch(std::exception e_range){
        continue;
      }
    }
  }

  void FileIO::initializeVariables(table<double>& var){
    //C-13 Set initial values
    for(int i=0;i<var["NL"];i++){
      change(var, fixIndex("LVSN",i), 0);
      change(var, fixIndex("STMS",i), 0);
      change(var, fixIndex("WLVS",i), 0);
      change(var, fixIndex("WSTM",i), 0);
      change(var, fixIndex("LFAR",i), 0);
    }

    for(int i=0;i<var["NF"];i++){
      change(var, fixIndex("FRTN",i), 0);
      change(var, fixIndex("WFRT",i), 0);
    }

    change(var, "PLSTN", var["PLSTNI"]);
    change(var, "CPOOL", 0.0);
    change(var, "LVSN(0)", var["LVSNI"]*var["PLM2"]);
    change(var, "BTOTNLV", var["LVSNi"]*var["plm2"]);
    change(var, "STMS(0)", var["LVSN(0)"]);
    change(var, "WLVS(0)", var["WLVSI"]*var["PLM2"]);
    change(var, "WSTM(0)", var["WLVS(0)"]*var["FRSTEM(0)"]);
    change(var, "LFAR(0)", var["LFARI"]*var["PLM2"]);
    change(var, "XLAI", var["LFAR(0)"]);
    change(var, "TOTWML", 0.0);
    change(var, "ATL", 0.0);
    change(var, "ATV", 0.0);
    change(var, "TOTWST", 0.0);
    change(var, "WTOTF", 0.0);
    change(var, "ASTOTL", var["XLAI"]);
    change(var, "WSTOTL", 0.0);
    change(var, "FWFR10", 0.0);
    change(var, "APFFW", 0.0);
    change(var, "ATT", 0.0);
  }

  void FileIO::inputWeather(table<double>& var, std::string fileName){
    //C-20 Get weather parameters from fileName and set
    //要修正
    std::ifstream ifs(fileName);
    std::vector<std::string> pair;
    std::string header[7] = {"KYR", "JUL", "SOLRAD", "TMAX", "TMIN", "RAIN", "PARO"};

    if(!ifs.is_open()){
      change(var, "Error", -1); //fault to open
      return;
    }
    
    //First data
    change(var, "XLAT", 31.2);
    change(var, "XLONG", 34.4);
    change(var, "PARFAC", 12.07);
    change(var, "PARDAT", 0.5); //?

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
          change(var, header[col], std::stod(pair[col]));
        }
      }catch(std::exception e_range){
        continue;
      }
    }
    change(var, "XLANG", var["SOLRAD"]*23.923);
    calc->sunrise(var);
    if(var["PARDAT"] < 0) var["PARO"] = var["XLANG"] / var["PARFAC"];
  }

  Calc::Calc(){}

  Calc::Calc(FileIO* pfileio){
    fileio = pfileio;
  }

  void Calc::sunrise(table<double>& var){
    var["HEMIS"] = var["XLAT"]/std::abs(var["XLAT"]);
    var["DEC"] = -1*var["HEMIS"]*23.4*std::cos(2*M_PI*(var["JUL"]+10)/365);
    var["SNDC"] = std::sin(var["RAD"]*var["DEC"]);
    var["CSDC"] = std::cos(var["RAD"]*var["DEC"]);
    var["SNLT"] = std::sin(var["RAD"]*var["XLAT"]);
    var["CSLT"] = std::cos(var["RAD"]*var["XLAT"]);
    var["SSIN"] = var["SNDC"]*var["SNLT"];
    var["CCOS"] = var["CSDC"]*var["CSLT"];
    var["TT"] = var["SSIN"]/var["CCOS"];
    var["AS"] = std::asin(var["TT"]);
    var["DAYL"] = 12*(M_PI+2*var["AS"])/M_PI;
    if(var["XLAY"] < 0) var["DAYL"] = 24 - var["DAYL"];
    var["SUP"] = 12 - var["DAYL"] / 2;
    var["SDN"] = 12 + var["DAYL"] / 2;
  }

  double Calc::tabex(table<double>& var, std::string val, std::string arg, double dummy, int k){
    //C-29
    int i = 1;
    for(i=1;i<k;i++){
      if(dummy <= var[fileio->fixIndex(arg, i)]) break;
    }
    //std::cout << "arg:" << arg << " val:" << val << std::endl;
    //std::cout << "arg:" << var[fileio->fixIndex(arg, i)] << " val:" << var[fileio->fixIndex(val, i)] << " dummy:" << dummy << std::endl;
    return (dummy - var[fileio->fixIndex(arg, i-1)]) * (var[fileio->fixIndex(val, i-1)] - var[fileio->fixIndex(val, i)]) / (var[fileio->fixIndex(arg, i)] - var[fileio->fixIndex(arg, i-1)]) + var[fileio->fixIndex(arg, i-1)];
  }

  void Calc::calcWeather(table<double>& var){
    //C-30 WCALC
    var["SDNT"] = var["SDN"];
    var["SUPT"] = var["SUP"];
    var["TMINT"] = var["TMIN"];
    var["TMAXT"] = var["TMAX"];

    if(var["JDAY"] <= 1){
      var["SDNY"] = var["SDN"];
      var["SUPY"] = var["SUP"];
      var["TMINY"] = var["TMIN"];
      var["TMAXY"] = var["TMAX"];
    }

    for(int i=0;i<25;i++){
      if(i < var["SUP"]+2.0){
        var["TAU"] = 3.1417 * (var["SDNY"] - var["SUPY"] - 2)/(var["SDNY"] - var["SUPY"]);
        var["TLIN"] = var["TMINY"] + ((var["TMAXY"] - var["TMINY"]) * std::sin(var["TAU"]));
        var["HDARK"] = 24 - var["SDNY"] + var["SUP"] + 2;
        var["SLOPE"] = (var["TLIN"] - var["TMIN"]) / var["HDARK"];
        var[fileio->fixIndex("THR", i)] = var["TLIN"] - var["SLOPE"] * (i + 24 - var["SDNY"]);
        continue;
      }else if(i > var["SDN"]){
        var["TAU"] = 3.1417 * (var["SDNY"] - var["SUP"] - 2)/(var["SDN"] - var["SUP"]);
        var["TLIN"] = var["TMIN"] + ((var["TMAX"] - var["TMIN"]) * std::sin(var["TAU"]));
        var["HDARK"] = 24 - var["SDN"] + var["SUPT"] + 2;
        var["SLOPE"] = (var["TLIN"] - var["TMINT"]) / var["HDARK"];
        var[fileio->fixIndex("THR", i)] = var["TLIN"] - var["SLOPE"] * (i + 24 - var["SDN"]);
        continue;
      }
    }
    var["SDNY"] = var["SDN"];
    var["SUPY"] = var["SUP"];
    var["TMINY"] = var["TMIN"];
    var["TMAXY"] = var["TMAX"];
    var["DL"] = var["SDN"] - var["SUP"];
    
    for(int i=0;i<25;i++){
      var[fileio->fixIndex("RAD", i)] = 0;
      if(i < var["SUP"] || i > var["SDN"]) continue;
      var[fileio->fixIndex("RAD", i)] = 3.1417 / (2 * var["DL"]) * var["SOLRAD"] * std::sin(3.1417 * (i - var["SUP"]) / var["DL"]);
    }
  }

  void Calc::ghouse(table<double>& var){
    double tmaxgh, tmingh, daytmp, co2avg;

    if(var["JDAY"] <= 0 && var["TFAST"] <= 0.0001){
      var["IENV"] = 0;
      char ans = 'N';
      std::cout << "Do you want constant environment? [Y/N]:" << std::endl;
      std::cin >> ans;
      if(ans == 'y' || ans == 'Y'){
        var["IENV"] = 1;
        std::cout << "Input the day and night temperatures and " <<
          "the hours of daytime temperature and CO2(e.g. 28.0 16.5 13.0 950.0):" << std::endl;
        tmaxgh = 28.0; tmingh = 16.5; daytmp = 13.0; co2avg = 950.0;
        //std::cin >> tmaxgh >> tmingh >> daytmp >> co2avg;
        var["TMAXGH"] = tmaxgh;
        var["TMINGH"] = tmingh;
        var["DAYTMP"] = daytmp;
        var["CO2AVG"] = co2avg;
        var["SUPGH"] = 12 - var["DAYTMP"]/2;
        var["SDNGH"] = 12 + var["DAYTMP"]/2;
      }
    }

    //If constant environment
    if(var["IENV"] == 0){
      for(int i=0;i<24;i++){
        var["XTMP"] = i;
        if(var["XTMP"] <= var["SUPGH"]){
          var[fileio->fixIndex("THR", i)] = var["TMINGH"];
        }else if(var["XTMP"] <= var["SDNGH"]){
          var[fileio->fixIndex("THR", i)] = var["TMAXGH"];
        }else{
          var[fileio->fixIndex("THR", i)] = var["TMINGH"];
        }
      }
    }

    var["CO2L"] = tabex(var, "CO2LT", "XCO2LT", var["TIME"], 6);
    var["TMPA"] = tabex(var, "THR", "HOURS", var["TFAST"], 25);
    var["RADA"] = tabex(var, "RAD", "HOURS", var["TFAST"], 25);

    if(var["IENV"] != 1) var["CO2AVG"] = var["CO2L"];

    var["TAVG"] = var["TMPA"];
    var["RADCAL"] = var["RADA"] * var["TRGH"];
    var["PAR"] = var["PARO"] * var["TRGH"];
    var["PPFD"] = var["RADA"] * 23.923 / var["PARFAC"] * 277.78 * var["TRGH"];
  }

  void Calc::devfast(table<double>& var){
    var["TSLAF"] = 1.0 + 0.045 * (24 - var["TMPA"]);
    var["CSLAF"] = 0;
    var["FCO2"] = 1.0;
    if(var["PPFD"] > 0.1){
      var["NCSLA"] = var["NCSLA"] + 1;
      var["CSLAF"] = 1.5 + var["CO2M"] * (var["CO2AVG"] - 350) / 600;
      var["FCO2"] = 1.0 + var["SCO2"] * (var["CO2AVG"] - 350) * var["AMIN1"] * std::min(1.0, 20/var["PLSTN"]);
    }
    var["TEMFCF"] = tabex(var, "GENTEM", "XTEM", var["TMPA"], 6);
    var["GENRF"] = std::min(std::min(var["EPS"], var["CLSDML"])/var["GENFA"],1.0) * var["TEMFCF"] * tabex(var, "GENRAT", "XGEN", var["PLSTN"], 6);
    var["RDVLVF"] = tabex(var, "RDVLVT", "XLV", var["TMPA"], 9) * var["SPTEL"] * var["FCO2"];
    var["RDVFRF"] = tabex(var, "RDVFRT", "XFRT", var["TMPA"], 9) * var["SPTEL"] * var["FCO2"];
    var["TTHF"] = 0;
    var["TTLF"] = 0;
    var["TTABF"] = 0;
    if(var["TMPA"] > var["THIGH"]) var["TTHF"] = var["TMPA"] - var["THIGH"];
    if(var["TMPA"] < var["TLOW"]) var["TTLF"] = var["TLOW"] - var["TMPA"];
    if(var["TMPA"] < var["TLOWAB"]) var["TTABF"] = var["TLOWAB"] - var["TMPA"];
  }

  void Calc::photo(table<double>& var){
    var["QE"] = 0.056;
    var["XK"] = 0.58;
    var["XM"] = 0.10;
    var["GPF"] = 0;
    var["TAU1"] = 0.06638 * var["TU1"];
    var["TAU2"] = 0.06638 * var["TU2"];
    var["PMAX"] = var["TAU1"] * var["CO2AVG"];
    if(var["CO2AVG"] > 350.0) var["PMAX"] = var["TAU1"] * 350 + var["TAU2"] * (var["CO2AVG"] - 350);
    var["AEF"] = tabex(var, "AEFT", "XAEFT", var["PLSTN"], 6);
    var["PMAX"] = var["PMAX"] * tabex(var, "PGRED", "TMPG", var["TMPA"], 8) * var["AEF"];
    
    if(var["PPFD"] >= 0.001){
      var["TOP"] = (1 - var["XM"]) * var["PMAX"] + var["QE"] * var["XK"] * var["PPFD"];
      var["BOT"] = (1 - var["XM"]) * var["PMAX"] + var["QE"] * var["XK"] * var["PPFD"] * std::exp(-1 * var["XK"] * var["ASTOTL"] * var["pltm2v"]);
      var["GPF"] = (var["PMAX"] / var["XK"]) * std::log(var["TOP"] / var["BOT"]) * 0.682 * 3.8016;
    }
  }

  void Calc::resp(table<double>& var){
    var["TEFF"] = std::pow(var["Q10"], 0.1 * var["TMPA"] - 2.0);
    var["MAINTF"] = var["TEFF"] * (var["RMRL"] * (var["TOTWST"] + var["WSTOTL"]) + var["RMRF"] * var["WTOTF"]);
    //std::cout << var["TEFF"] << " " << var["RMRL"] << " " << var["TOTWST"] << " " << var["WSTOTL"] << " " << var["WTOTF"] << std::endl;
  }
}