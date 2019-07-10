#include "lib.hpp"

namespace tomgro{
  using param = std::pair<std::string, double>;

  std::vector<std::string> FileIO::split(std::string& input, char delimiter){
    std::istringstream stream(input);
    std::string field;
    std::vector<std::string> result;
    while (getline(stream, field, delimiter)) {
        result.push_back(field);
    }
    return result;
  }

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

    var["NEW"] = 0; //0:First work
    
    //First data
    change(var, "XLAT", 31.2);
    change(var, "XLONG", 34.4);
    change(var, "PARFAC", 12.07);
    change(var, "PARDAT", 0.5); //?

    while (!ifs.eof()){
      try{
        std::string buffer;
        ifs >> buffer;
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

  void FileIO::output(table<double>& var){
    //C-23
    var["TOTNLV/PLM2"] = var["TOTNLV"] / var["PLM2"];
    std::string data1[] = {"DATE", "PLSTN", "XLAI", "TOTNLV/PLM2", "TOTWML", 
      "TOTNF", "TOTWMF", fixIndex("FRTN", var["NF"]), fixIndex("WFRT", var["NF"]), "XSLA", 
      "ASTOTL", "GP", "MAINT", "TRCDRW", "RCDRW", "FWFR10", "TOTWST",
      "APFFW", "TOTVW", "TOTDW", "DLN", "CLSDML", "TEMFAC", "ATL", "ATV", "ATT"};
    std::string data2[] = {"DATE", "ATV", "LVSN(4)", "LVSN(9)", "LVSN(14)", "LVSN(19)", 
      "FRTN(0)", "FRTN(4)", "FRTN(9)", "FRTN(14)", "FRTN(19)", "ABFN"};

    for(std::string data : data1){
      //std::cout << data << ":" << var[data] << " ";
    }
    //std::cout << "\n";
    for(std::string data : data2){
      //std::cout << data << ":" << var[data] << " ";
    }
    //std::cout << "\n";
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
   //std::cout << "\tnannanTABEX:dummy:" << dummy << ", i:" << i << std::endl;
    //std::cout << "\tnannanTABEX(" << fileio->fixIndex(val, i) << "&"<< fileio->fixIndex(arg, i) << "):" << var[fileio->fixIndex(val, i-1)] << " " << var[fileio->fixIndex(val, i)] << " " << var[fileio->fixIndex(arg, i-1)] << " " << var[fileio->fixIndex(arg, i)] << std::endl;
    double ans = (dummy - var[fileio->fixIndex(arg, i-1)]) * (var[fileio->fixIndex(val, i-1)] - var[fileio->fixIndex(val, i)]) / (var[fileio->fixIndex(arg, i)] - var[fileio->fixIndex(arg, i-1)]) + var[fileio->fixIndex(arg, i-1)];
    //std::cout << "\tnannanTABEXans:" << ans << std::endl;
    if(ans == 0) ans = 0.0000001;
    return ans;
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
      ans = 'Y';
      //std::cin >> ans;
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
    var["GENRF"] = std::min(std::min(var["EPS"], var["CLSDML"])/var["GENFAC"],1.0) * var["TEMFCF"] * tabex(var, "GENRAT", "XGEN", var["PLSTN"], 6);
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

  void Calc::dmrate(table<double>& var){
    //C-8
    var["PARSLA"] = 1.0 - tabex(var, "PART", "XPART", var["PAR"], 5);
    var["ESLA"] = var["STDSLA"] * var["PARSLA"] / (var["TSLA"] * var["CSLA"]);
    var["ESLA"] = std::min(0.018, var["ESLA"]);
    var["ESLA"] = std::min(var["SLAMX"], var["ESLA"]);
    var["TRCDRW"] = (var["GP"] / var["PLTM2V"] - var["MAINT"]) * var["GREF"];
    var["TRCDRW"] = std::min(var["TRCDRW"], 0.0);
    var["RCDRW"] = var["TRCDRW"] * (1.0 - tabex(var, "PROOT", "XROOT", var["PLSTN"], 6.0)) * (std::min(std::max(var["EPS"], var["CLSDML"]) / var["ZBENG"], 1.0) * var["TEMFAC"]);
    var["PTNLVS"] = 0.0;
    var["PTNSTM"] = 0.0;
    for(int i=0;i<var["NL"];i++){
      var["XBOX"] = (i+1) * 100.0 / var["NL"];
      var[fileio->fixIndex("RCLFA", i)] = var[fileio->fixIndex("LVSN", i)] * tabex(var, "POL", "BOX", var["XBOX"], 10.0) * var["TEMFAC"] * var["FCO2D"];
      var["FRPT"] = tabex(var, "FRPET", "BOX", var["XBOX"], 10);
      var["FRST"] = tabex(var, "FRSTEM", "BOX", var["XBOX"], 10);
      var[fileio->fixIndex("PNLVS", i)] = (var[fileio->fixIndex("RCLFA", i)] / (tabex(var, "ASLA", "BOX", var["XBOX"], 10) * var["ESLA"])) * (1.0 + var["FRPT"]);
      var["PTNLVS"] = var["PTNLVS"] + var[fileio->fixIndex("PNLVS", i)];
      //std::cout << "(*>△<)nannan<" << var[fileio->fixIndex("LVSN", i)]+var["PSP"] << std::endl;
      var[fileio->fixIndex("PNSTM", i)] = var[fileio->fixIndex("PNLVS", i)] / (var[fileio->fixIndex("LVSN", i)] + var["EPS"]) * var["FRST"] * var[fileio->fixIndex("STMS", i)];
      var["PTNSTM"] = var["PTNSTM"] + var[fileio->fixIndex("PNSTM", i)];
    }
    var["PTNFRT"] = 0.0;
    for(int i=0;i<var["NF"];i++){
      var["ZZX"] = std::min(1.0, std::max(var["EPS"], 2.0 - var[fileio->fixIndex("AVWF", i)] / var["MAVF"]));
      var["XBOX"] = (i+1) * 100.0 / var["NF"];
      var[fileio->fixIndex("PNFRT", i)] = var[fileio->fixIndex("FRTN", i)] * tabex(var, "POF", "BOX", var["XBOX"], 10) * var["TEMFAC"] * var["FCO2D"] * var["ZZX"];
      var["PTNFRT"] = var["PTNFRT"] + var[fileio->fixIndex("PNFRT", i)];
    }  
    var["PNGP"] = var["PTNLVS"] + var["PTNFRT"] + var["PTNSTM"];
    
    var["TOTDML"] = std::min(var["RCDRW"] * var["PTNLVS"] / (var["PNGP"] + var["EPS"]), var["PTNLVS"]);
    var["TOTDMS"] = std::min(var["RCDRW"] * var["PTNSTM"] / (var["PNGP"] + var["EPS"]), var["PTNSTM"]);
    var["TOTDMF"] = std::min(var["RCDRW"] * var["PTNFRT"] / (var["PNGP"] + var["EPS"]), var["PTNFRT"]);
    var["TOPGR"] = var["TOTDMF"] + var["TOTDML"] + var["TOTDMS"];
    var["EXCESS"] = var["RCDRW"] - var["TOPGR"];
    var["CLSDMF"] = 1.0;
    if(var["PTNFRT"] > 0.0) var["CLSDMF"] = var["TOTDMF"] / (var["PTNFRT"] + var["EPS"]);
    var["CLSDML"] = var["TOTDML"] / (var["PTNLVS"] + var["EPS"]);
    for(int i=0;i<var["NL"];i++){
      var[fileio->fixIndex("RCWLV", i)] = var["TOTDML"] * var[fileio->fixIndex("PNLVS", i)] / (var["PTNLVS"] + var["EPS"]);
      var[fileio->fixIndex("RCWST", i)] = var["TOTDMS"] * var[fileio->fixIndex("PNSTM", i)] / (var["PTNSTM"] + var["EPS"]);
      var["XBOX"] = (i+1) * 100.0 / var["NL"];
      var["FRPT"] = tabex(var, "FRPET", "BOX", var["XBOX"], 10);
      var[fileio->fixIndex("RCLFA", i)] = var[fileio->fixIndex("RCWLV", i)] * tabex(var, "ASLA", "BOX", var["XBOX"], 10) * var["ESLA"] / (1.0 + var["FRPT"]);
    }
    for(int i=0;i<var["NF"];i++){
      var[fileio->fixIndex("RCWFR", i)] = var["TOTDMF"] * var[fileio->fixIndex("PNFRT", i)] / (var["PTNFRT"] + var["EPS"]);
    }
  }

  void Calc::devrate(table<double>& var){
    var["TPLA"] = 0.0;
    if(var["PLSTN"] > var["FTRUSN"]) var["TPLA"] = var["TPL"];
    var["RCNL"] = var["PLM2"] * var["GENR"] / (1.0 + var["TPLA"]);
    var["RCST"] = var["PLM2"] * var["GENR"];
    var["RCNF"] = var["GENR"] * tabex(var, "FPN", "XFPN", var["PLSTN"] - var["FRLG"], 10) * var["PLM2"];
    var["RCNF"] = var["RCNF"] * std::max(1.0 - var["TTH"] / var["TTMX"], 0.0) * std::max(1.0 + var["TTL"] / var["TTMN"], 0.0);
    var["PUSHL"] = var["RDVLV"] * var["NL"];
    var["PUSHM"] = var["RDVFR"] * var["NF"];
  }

  void Calc::losrate(table<double>& var){
    var["ABNF"] = 0.0;
    if(var["TOTDMF"] >= var["EPS"]){
      var["FABOR"] = std::min(1.0, 2.0 - var["ABORMX"] * var["CLSDML"]);
      var["FABOR"] = std::max(0.0, var["FABOR"]);
      var["TABOR"] = std::min(1.0, std::max(0.0, var["TTAB"]/var["TABK"]));
      var["ABNF"] = var["FABOR"] * var["RCNF"] / var["PLTM2V"] + var["TABOR"] * var["RCNF"] / var["PLTM2V"];
    }
    var[fileio->fixIndex("DEAR", var["NL"])] = 0.0;
    if(var["XLAI"] * var["PLTMV2"] > var["XLAIM"])  var[fileio->fixIndex("DEAR", var["NL"])] = var["XMRDR"] * std::min(var[fileio->fixIndex("LFAR", var["NL"])], (var["XLAI"] * var["PLTM2V"] - var["XLAIM"]) / var["PLTM2V"]);
    var[fileio->fixIndex("DEAR", var["NL"])] = std::max(0.0, var[fileio->fixIndex("DEAR", var["NL"])]);
    var["DATEZ"] = tabex(var, "DISDAT", "XDISDAT", var["TIME"], 12);

    for(int i=0;i<var["NL"]-1;i++){
      var["XBOX"] = (i+1) * 100.0 / var["NL"];
      var[fileio->fixIndex("DEAR", i)] = tabex(var, "DIS", "BOX", var["XBOX"], 10) * var["DATEZ"];
    }

    for(int i=0;i<var["NL"];i++){
      var[fileio->fixIndex("DENLR", i)] = var[fileio->fixIndex("LVSN", i)] * var[fileio->fixIndex("DEAR", i)];
      var[fileio->fixIndex("DEWLR", i)] = var[fileio->fixIndex("DENLR", i)] * var[fileio->fixIndex("AVWL", i)];
      var[fileio->fixIndex("DELAR", i)] = var[fileio->fixIndex("DEAR", i)] * var[fileio->fixIndex("LFAR", i)];
    }

    for(int i=0;i<var["NF"];i++){
      var["XBOX"] = (i+1) * 100.0 / var["NF"];
      var[fileio->fixIndex("DEAF", i)] = tabex(var, "DISF", "BOX", var["XBOX"], 10) * var["DATEZ"];
      var[fileio->fixIndex("DENFR", i)] = var[fileio -> fixIndex("FRTN", i)] * var[fileio -> fixIndex("DEAF", i)];
      var[fileio->fixIndex("DEWFR", i)] = var[fileio -> fixIndex("DENFR", i)] * var[fileio -> fixIndex("AVWF", i)];
    }
  }

  void Calc::intgrat(table<double>& var){
    //C-16
    var["CPOOL"] = var["CPOOL"] + (var["GP"] - var["RCDRW"] / var["GREF"] - var["MAINT"]) * var["DELT"];
    var["PLSTN"] = var["PLSTN"] + var["GENR"] * var["DELT"];
    var[fileio->fixIndex("LVSN", var["NL"])] = var[fileio->fixIndex("LVSN", var["NL"])] + (var["PUSHL"] * var[fileio->fixIndex("LVSN", var["NL"]-1)] - var[fileio->fixIndex("DENLW", var["NL"])]) * var["DELT"];
    var[fileio->fixIndex("WLVS", var["NL"])] = var[fileio->fixIndex("WLVS", var["NL"])] + (var["PUSHL"] * var[fileio->fixIndex("WLVS", var["NL"]-1)] - var[fileio->fixIndex("DEWLR", var["NL"])]) * var["DELT"];
    var[fileio->fixIndex("LFAR", var["NL"])] = var[fileio->fixIndex("LFAR", var["NL"])] + (var["PUSHL"] * var[fileio->fixIndex("LFAR", var["NL"]-1)] - var[fileio->fixIndex("DELAR", var["NL"])]) * var["DELT"];
    var[fileio->fixIndex("STMS", var["NL"])] = var[fileio->fixIndex("STMS", var["NL"])] + var["PUSHL"] * var[fileio->fixIndex("STMS", var["NL"]-1)] * var["DELT"];
    var[fileio->fixIndex("WSTM", var["NL"])] = var[fileio->fixIndex("WSTM", var["NL"])] + var["PUSHL"] * var[fileio->fixIndex("WSTM", var["NL"]-1)] * var["DELT"];

    for(int i=1;i<var["NL"]-1;i++){
      int ii = var["NL"] - i + 1;
      var[fileio->fixIndex("LVSN", ii)] = var[fileio->fixIndex("LVSN", ii)] + var["PUSHL"] * (var[fileio->fixIndex("LVSN", ii-1)] - var[fileio->fixIndex("LVSN", ii)]) * var["DELT"] - var[fileio->fixIndex("DENLR", ii)] * var["DELT"];
      var[fileio->fixIndex("STMS", ii)] = var[fileio->fixIndex("STMS", ii)] + var["PUSHL"] * (var[fileio->fixIndex("STMS", ii-1)] - var[fileio->fixIndex("STMS", ii)]) * var["DELT"];
      var[fileio->fixIndex("WLVS", ii)] = var[fileio->fixIndex("WLVS", ii)] + var["PUSHL"] * (var[fileio->fixIndex("WLVS", ii-1)] - var[fileio->fixIndex("WLVS", ii)]) + var[fileio->fixIndex("RCWLV", ii)] * var["DELT"] - var[fileio->fixIndex("DEWLR", ii)] * var["DELT"];
      var[fileio->fixIndex("WSTM", ii)] = var[fileio->fixIndex("WSTM", ii)] + (var["PUSHL"] * (var[fileio->fixIndex("WSTM", ii-1)] - var[fileio->fixIndex("WSTM", ii)]) + var[fileio->fixIndex("RCWST", ii)]) * var["DELT"];
      var[fileio->fixIndex("LFAR", ii)] = var[fileio->fixIndex("LFAR", ii)] + (var["PUSHL"] * (var[fileio->fixIndex("LFAR", ii-1)] - var[fileio->fixIndex("LFAR", ii)]) + var[fileio->fixIndex("RCWST", ii)]) * var["DELT"] - var[fileio->fixIndex("DELAR", ii)] * var["DELT"];
    }

    var["LVSN(0)"] = (var["RCNL"] - var["PUSHL"] * var["LVSN(0)"]) * var["DELT"] + var["LVSN(0)"] - var["DENLR(0)"] * var["DELT"];
    var["STMS(0)"] = var["STMS(0)"] + (var["RCST"] - var["PUSHL"] * var["STMS(0)"]) * var["DELT"];
    var["WLVS(0)"] = (var["RCNL"] * var["WPLI"] - var["PUSHL"] * var["WLVS(0)"] + var["RCWLV(0)"]) * var["DELT"] + var["WLVS(0)"] - var["DEWLR(0)"] * var["DELT"];
    var["WSTM(0)"] = var["WSTM(0)"] + (var["RCST"] * var["WPLI"] * var["FRSTEM(0)"] - var["PUSHL"] * var["WSTM(0)"] + var["RCWST(0)"]) * var["DELT"];
    var["FRPT"] = 1.0 + var["FRPET(0)"];
    var["LFAR(0)"] = (var["RCNL"] * var["WPLI"] * var["ESLA"] * var["ASLA(0)"] / var["FRPT"] - var["PUSHL"] * var["LFAR(0)"] + var["RCLFA(0)"]) * var["DELT"] + var["LFAR(0)"] - var["DELAR(0)"] * var["DELT"];

    var[fileio->fixIndex("FRTN", var["NF"])] = var[fileio->fixIndex("FRTN", var["NF"])] + (var["PUSHM"] * var[fileio->fixIndex("FRTN", var["NF"]-1)] - var[fileio->fixIndex("DENFR", var["NF"])]) * var["DELT"];
    var[fileio->fixIndex("WFRT", var["NF"])] = var[fileio->fixIndex("WFRT", var["NF"])] + (var["PUSHM"] * var[fileio->fixIndex("WFRT", var["NF"]-1)] - var[fileio->fixIndex("DEWFR", var["NF"])]) * var["DELT"];

    for(int i=1;i<var["NF"]-1;i++){
      int ii = var["NF"] - i + 1;
      var[fileio->fixIndex("FRTN", ii)] = var[fileio->fixIndex("FRTN", ii)] + var["PUSHM"] * (var[fileio->fixIndex("FRTN", ii-1)] - var[fileio->fixIndex("FRTN", ii)]) * var["DELT"] - var[fileio->fixIndex("DENFR", ii)] * var["DELT"];
      var[fileio->fixIndex("WFRT", ii)] = var[fileio->fixIndex("WFRT", ii)] + (var["PUSHM"] * (var[fileio->fixIndex("WFRT", ii-1)] - var[fileio->fixIndex("WFRT", ii)]) + var[fileio->fixIndex("RCWFR", ii)]) * var["DELT"] - var[fileio->fixIndex("DEWFR", ii)] * var["DELT"];
    }
    var["FRTN(0)"] = (var["RCNF"] - var["ABNF"] - var["PUSHM"] * var["FRTN(0)"]) * var["DELT"] + var["FRTN(0)"] - var["DENFR(0)"] * var["DELT"];
    var["WFRT(0)"] = ((var["RCNF"] - var["ABNF"]) * var["WPFI"] - var["PUSHM"] * var["WFRT(0)"] + var["RCWFR(0)"]) * var["DELT"] + var["WFRT(0)"] - var["DEWFR(0)"] * var["DELT"];
    
    var["XLAI"] = 0.0;
    var["TWTLAI"] = 0.0;
    var["TOTNLV"] = 0.0;
    var["TOTWML"] = 0.0;
    var["TOTNST"] = 0.0;
    var["TOTWST"] = 0.0;
    var["ATV"] = 0.0;

    for(int i=0;i<var["NL"];i++){
      var[fileio->fixIndex("AVWL", i)] = var[fileio->fixIndex("WLVS", i)] / (var[fileio->fixIndex("LVSN", i)] + var["EPS"]);
      var["XLAI"] = var["XLAI"] + var[fileio->fixIndex("LFAR", i)];
      var["TOTNLV"] = var["TOTNLV"] + var[fileio->fixIndex("LVSN", i)];
      var["TOTWML"] = var["TOTWML"] + var[fileio->fixIndex("WLVS", i)];
      var["ATL"] = var["ALT"] + var[fileio->fixIndex("DEWLR", i)] * var["DELT"];
      var["XBOX"] = (i+1) * 100 / var["NL"];
      var["FRPT"] = tabex(var, "FRPET", "BOX", var["XBOX"], 10);
      var["TWTLAI"] = var["TWTLAI"] + var[fileio->fixIndex("WLVS", i)] / (1.0 + var["FRPT"]);
      var["TOTNST"] = var["TOTNST"] + var[fileio->fixIndex("STMS", i)];
      var["TOTWST"] = var["TOTWST"] + var[fileio->fixIndex("WSTM", i)];
    }

    var["XSLA"] = var["XLAI"] / (var["TWTLAI"] + var["EPS"]) * 10000.0;
    var["TOTWMF"] = 0.0;
    var["TOTNF"] = 0.0;

    for(int i=0;i<var["NF"];i++){
      var[fileio->fixIndex("AVWL", i)] = var[fileio->fixIndex("WFRT", i)] / (var[fileio->fixIndex("FRTN", i)] + var["EPS"]);
      var["TOTWMF"] = var["TOTWMF"] + var[fileio->fixIndex("WFRT", i)];
      var["TOTNF"] = var["TOTNF"] + var[fileio->fixIndex("FRTN", i)];
    }

    var["WTOTF"] = var["TOTWMF"] - var[fileio->fixIndex("WFRT", var["NF"])];
    var["TOTGF"] = var["TOTNF"] - var[fileio->fixIndex("FRTN", var["NF"])];
    var["BTOTNLV"] = var["BTOTNLV"] + var["RCNL"] * var["DELT"];
    var["DLN"] = (var["BTOTNLV"] - var["TOTNLV"]) / var["PLM2"];
    var["TOTGL"] = 0.0;
    var["ASTOTL"] = 0.0;
    var["WSTOTL"] = var["TOTWML"] - var[fileio->fixIndex("WLVS", var["NL"])];
    var["TOTGL"] = var["TOTNLV"] - var[fileio->fixIndex("LVSN", var["NL"])];
    var["ASTOTL"] = var["XLAI"] - var[fileio->fixIndex("LFAR", var["NL"])];
    var["TOTST"] = var["TOTNST"] - var[fileio->fixIndex("STMS", var["NL"])];
    var["WSTOTS"] = var["TOTWST"] - var[fileio->fixIndex("WSTM", var["NL"])];
    var["TOTDW"] = var["TOTWMF"] + var["TOTWML"] + var["TOTWST"];
    var["TOTVM"] = var["TOTWML"] + var["TOTWST"];
    var["ATV"] = var["TOTWML"] + var["TOTWST"] + var["ATL"];
    var["ATT"] = var["ATV"] + var["TOTWMF"];
    var["TOTNU"] = var["TOTNF"] + var["TOTNLV"];
    var["NGP"] = var["TOTGL"] + var["TOTGF"] + var["TOTST"]; 
    var["RVRW"] = var["TOTWMF"] / (var["TOTWML"] + var["EPS"]);
    var["RRRW"] = var["TOTWMF"] / (var["TOTDW"] + var["EPS"]);
    var["RVRN"] = var["TOTNF"] / (var["TOTNLV"] + var["EPS"]);
    var["RTRN"] = var["TOTNF"] / (var["TOTNU"] + var["EPS"]);
    var["AVWMF"] = var["TOTWMF"] / (var["TOTNF"] + var["EPS"]);
    var["AVWML"] = var["TOTWML"] / (var["TOTNLV"] + var["EPS"]);
    var["DMCF84"] = tabex(var, "DMC84T", "XDMC", var["TIME"], 6);
    var["FWFR10"] = var["FWFR10"] + (var["PUSHM"] * var[fileio->fixIndex("WFRT", var["NF"]-1)] * var["DELT"]) * 100.0 / var["DMCF84"];
    var["APFFW"] = ((var["PUSHM"] * std::min(var[fileio->fixIndex("WFRT", var["NF"]-1)], 0.0) * var["DELT"]) * 100.0 / var["DMCF84"]) / ((var["PUSHM"] * var[fileio->fixIndex("FRTN", var["NF"]-1)] * var["DELT"]) + var["EPS"]);
  }
}