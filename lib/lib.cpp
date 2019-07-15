#include "lib.hpp"
#define ANNA std::cout << "(*>△<)<"

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

  void FileIO::change(table& var, std::string name, double val){
    var[name].value = val;
  }

  void FileIO::test(){
    std::cout << "Hello World!" << std::endl;
  }

  void FileIO::inputData(table& var, std::string fileName){
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

  void FileIO::initializeVariables(table& var){
    //C-13 Set initial values
    var["DTFAST"].value = 1.0 / var["NFAST"].value;
    for(int i=0;i<var["NL"].value;i++){
      change(var, fixIndex("LVSN",i), 0);
      change(var, fixIndex("STMS",i), 0);
      change(var, fixIndex("WLVS",i), 0);
      change(var, fixIndex("WSTM",i), 0);
      change(var, fixIndex("LFAR",i), 0);
    }

    for(int i=0;i<var["NF"].value;i++){
      change(var, fixIndex("FRTN",i), 0);
      change(var, fixIndex("WFRT",i), 0);
    }

    change(var, "PLSTN", var["PLSTNI"].value);
    change(var, "CPOOL", 0.0);
    change(var, "LVSN(0)", var["LVSNI"].value*var["PLM2"].value);
    change(var, "BTOTNLV", var["LVSNI"].value*var["PLM2"].value);
    change(var, "STMS(0)", var["LVSN(0)"].value);
    change(var, "WLVS(0)", var["WLVSI"].value*var["PLM2"].value);
    change(var, "WSTM(0)", var["WLVS(0)"].value*var["FRSTEM(0)"].value);
    change(var, "LFAR(0)", var["LFARI"].value*var["PLM2"].value);
    change(var, "XLAI", var["LFAR(0)"].value);
    change(var, "TOTWML", 0.0);
    change(var, "ATL", 0.0);
    change(var, "ATV", 0.0);
    change(var, "TOTWST", 0.0);
    change(var, "WTOTF", 0.0);
    change(var, "ASTOTL", var["XLAI"].value);
    change(var, "WSTOTL", 0.0);
    change(var, "FWFR10", 0.0);
    change(var, "APFFW", 0.0);
    change(var, "ATT", 0.0);
  }

  void FileIO::inputWeather(table& var, std::string fileName){
    //C-20 Get weather parameters from fileName and set
    //要修正
    std::ifstream ifs(fileName);
    std::vector<std::string> pair;
    std::string header[7] = {"KYR", "JUL", "SOLRAD", "TMAX", "TMIN", "RAIN", "PARO"};

    if(!ifs.is_open()){
      change(var, "Error", -1); //fault to open
      return;
    }

    var["NEW"].value = 0; //0:First work
    
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
    change(var, "XLANG", var["SOLRAD"].value*23.923);
    calc->sunrise(var, "JUL", "SUP", "SDN", "XLAT", "XLONG");
    if(var["PARDAT"].value < 0) var["PARO"].value = var["XLANG"].value / var["PARFAC"].value;
  }

  void FileIO::output(table& var){
    //C-23
    var["TOTNLV/PLM2"].value = var["TOTNLV"].value / var["PLM2"].value;
    std::string data1[] = {"DATE", "PLSTN", "XLAI", "TOTNLV/PLM2", "TOTWML", 
      "TOTNF", "TOTWMF", fixIndex("FRTN", var["NF"].value), fixIndex("WFRT", var["NF"].value), "XSLA", 
      "ASTOTL", "GP", "MAINT", "TRCDRW", "RCDRW", "FWFR10", "TOTWST",
      "APFFW", "TOTVW", "TOTDW", "DLN", "CLSDML", "TEMFAC", "ATL", "ATV", "ATT"};
    std::string data2[] = {"DATE", "ATV", "LVSN(4)", "LVSN(9)", "LVSN(14)", "LVSN(19)", 
      "FRTN(0)", "FRTN(4)", "FRTN(9)", "FRTN(14)", "FRTN(19)", "ABFN"};

    
    for(std::string data : data1){
      std::cout << data << ":" << var[data].value << " ";
    }
    std::cout << "\n";
    for(std::string data : data2){
      std::cout << data << ":" << var[data].value << " ";
    }
    std::cout << "\n";
    
  }

  Calc::Calc(){}

  Calc::Calc(FileIO* pfileio){
    fileio = pfileio;
  }

  void Calc::sunrise(table& var, std::string IJUL, std::string XSNUP, std::string XSNDN, std::string XLAT, std::string XLONG){
    var["PI"].value = M_PI;
    var["RAD"].value = M_PI/180.0;
    var["HEMIS"].value = var[XLAT].value/std::abs(var[XLAT].value);
    var["DEC"].value = -1*var["HEMIS"].value*23.4*std::cos(2*M_PI*(var["JUL"].value+10)/365);
    var["SNDC"].value = std::sin(var["RAD"].value*var["DEC"].value);
    var["CSDC"].value = std::cos(var["RAD"].value*var["DEC"].value);
    var["SNLT"].value = std::sin(var["RAD"].value*var[XLAT].value);
    var["CSLT"].value = std::cos(var["RAD"].value*var[XLAT].value);
    var["SSIN"].value = var["SNDC"].value*var["SNLT"].value;
    var["CCOS"].value = var["CSDC"].value*var["CSLT"].value;
    var["TT"].value = var["SSIN"].value/var["CCOS"].value;
    var["AS"].value = std::asin(var["TT"].value);
    var["DAYL"].value = 12.0 * (M_PI + 2.0 *var["AS"].value)/M_PI;
    if(var[XLAT].value < 0) var["DAYL"].value = 24 - var["DAYL"].value;
    var[XSNUP].value = 12.0 - var["DAYL"].value / 2.0;
    var[XSNDN].value = 12.0 + var["DAYL"].value / 2.0;
  }

  double Calc::tabex(table& var, std::string val, std::string arg, double dummy, int k){
    //C-29
    int i = 1;
    for(i=1;i<k;i++){
      if(dummy <= var[fileio->fixIndex(arg, i)].value) break;
    }
    if(val == "RAD"){
      //std::cout << "\te_nannanTABEX:dummy:" << dummy << ", i:" << i << std::endl;
      //std::cout << "\te_nannanTABEX(" << fileio->fixIndex(val, i) << "&"<< fileio->fixIndex(arg, i) << "):" << var[fileio->fixIndex(val, i-1)].value << " " << var[fileio->fixIndex(val, i)].value << " " << var[fileio->fixIndex(arg, i-1)].value << " " << var[fileio->fixIndex(arg, i)].value << std::endl;
    }
    double ans = (dummy - var[fileio->fixIndex(arg, i-1)].value) * (var[fileio->fixIndex(val, i-1)].value - var[fileio->fixIndex(val, i)].value) / (var[fileio->fixIndex(arg, i)].value - var[fileio->fixIndex(arg, i-1)].value) + var[fileio->fixIndex(val, i-1)].value;
    if(ans == 0) ans = 0.0000001;
    return ans;
  }

  void Calc::calcWeather(table& var){
    //C-30 WCALC
    var["SDNT"].value = var["SDN"].value;
    var["SUPT"].value = var["SUP"].value;
    var["TMINT"].value = var["TMIN"].value;
    var["TMAXT"].value = var["TMAX"].value;

    if(var["JDAY"].value <= 1){
      var["SDNY"].value = var["SDN"].value;
      var["SUPY"].value = var["SUP"].value;
      var["TMINY"].value = var["TMIN"].value;
      var["TMAXY"].value = var["TMAX"].value;
    }

    for(int i=0;i<25;i++){
      if(i < var["SUP"].value+2.0){
        var["TAU"].value = 3.1417 * (var["SDNY"].value - var["SUPY"].value - 2)/(var["SDNY"].value - var["SUPY"].value);
        var["TLIN"].value = var["TMINY"].value + ((var["TMAXY"].value - var["TMINY"].value) * std::sin(var["TAU"].value));
        var["HDARK"].value = 24 - var["SDNY"].value + var["SUP"].value + 2;
        var["SLOPE"].value = (var["TLIN"].value - var["TMIN"].value) / var["HDARK"].value;
        var[fileio->fixIndex("THR", i)].value = var["TLIN"].value - var["SLOPE"].value * (i + 24 - var["SDNY"].value);
        continue;
      }else if(i > var["SDN"].value){
        var["TAU"].value = 3.1417 * (var["SDNY"].value - var["SUP"].value - 2)/(var["SDN"].value - var["SUP"].value);
        var["TLIN"].value = var["TMIN"].value + ((var["TMAX"].value - var["TMIN"].value) * std::sin(var["TAU"].value));
        var["HDARK"].value = 24 - var["SDN"].value + var["SUPT"].value + 2;
        var["SLOPE"].value = (var["TLIN"].value - var["TMINT"].value) / var["HDARK"].value;
        var[fileio->fixIndex("THR", i)].value = var["TLIN"].value - var["SLOPE"].value * (i + 24 - var["SDN"].value);
        continue;
      }
    }
    var["SDNY"].value = var["SDN"].value;
    var["SUPY"].value = var["SUP"].value;
    var["TMINY"].value = var["TMIN"].value;
    var["TMAXY"].value = var["TMAX"].value;
    var["DL"].value = var["SDN"].value - var["SUP"].value;
    
    for(int i=0;i<25;i++){
      var[fileio->fixIndex("RAD", i)].value = 0;
      //std::cout << "(e e)" << var["SUP"].value << "," << var["SDN"].value << std::endl;
      if(i < var["SUP"].value || i > var["SDN"].value) continue;
      var[fileio->fixIndex("RAD", i)].value = 3.1417 / (2 * var["DL"].value) * var["SOLRAD"].value * std::sin(3.1417 * (i - var["SUP"].value) / var["DL"].value);
      //std::cout << "e" << i << ":"  << var[fileio->fixIndex("RAD", i)].value << std::endl;
    }
  }

  void Calc::ghouse(table& var){
    double tmaxgh, tmingh, daytmp, co2avg;

    if(var["JDAY"].value <= 0 && var["TFAST"].value <= 0.0001){
      var["IENV"].value = 0;
      char ans = 'N';
      std::cout << "Do you want constant environment? [Y/N]:" << std::endl;
      ans = 'Y';
      //std::cin >> ans;
      if(ans == 'y' || ans == 'Y'){
        var["IENV"].value = 1;
        std::cout << "Input the day and night temperatures and " <<
          "the hours of daytime temperature and CO2(e.g. 28.0 16.5 13.0 950.0):" << std::endl;
        tmaxgh = 28.0; tmingh = 16.5; daytmp = 13.0; co2avg = 950.0;
        //std::cin >> tmaxgh >> tmingh >> daytmp >> co2avg;
        var["TMAXGH"].value = tmaxgh;
        var["TMINGH"].value = tmingh;
        var["DAYTMP"].value = daytmp;
        var["CO2AVG"].value = co2avg;
        var["SUPGH"].value = 12 - var["DAYTMP"].value/2;
        var["SDNGH"].value = 12 + var["DAYTMP"].value/2;
      }
    }

    //If constant environment
    if(var["IENV"].value != 0){
      for(int i=0;i<24;i++){
        var["XTMP"].value = i;
        if(var["XTMP"].value <= var["SUPGH"].value){
          var[fileio->fixIndex("THR", i)].value = var["TMINGH"].value;
        }else if(var["XTMP"].value <= var["SDNGH"].value){
          var[fileio->fixIndex("THR", i)].value = var["TMAXGH"].value;
        }else{
          var[fileio->fixIndex("THR", i)].value = var["TMINGH"].value;
        }
      }
    }

    var["CO2L"].value = tabex(var, "CO2LT", "XCO2LT", var["TIME"].value, 6);
    var["TMPA"].value = tabex(var, "THR", "HOURS", var["TFAST"].value, 25);
    var["RADA"].value = tabex(var, "RAD", "HOURS", var["TFAST"].value, 25);

    if(var["IENV"].value != 1) var["CO2AVG"].value = var["CO2L"].value;

    var["TAVG"].value = var["TMPA"].value;
    var["RADCAL"].value = var["RADA"].value * var["TRGH"].value;
    var["PAR"].value = var["PARO"].value * var["TRGH"].value;
    var["PPFD"].value = var["RADA"].value * 23.923 / var["PARFAC"].value * 277.78 * var["TRGH"].value;
  }

  void Calc::devfast(table& var){
    if(var["CLSDML"].value == 1e10) var["CLSDML"].value = 0.5; //仮
    var["TSLAF"].value = 1.0 + 0.045 * (24 - var["TMPA"].value);
    var["CSLAF"].value = 0;
    var["FCO2"].value = 1.0;
    if(var["PPFD"].value > 0.1){
      var["NCSLA"].value = var["NCSLA"].value + 1;
      var["CSLAF"].value = 1.5 + var["CO2M"].value * (var["CO2AVG"].value - 350) / 600;
      var["FCO2"].value = 1.0 + var["SCO2"].value * (var["CO2AVG"].value - 350) * std::min(1.0, 20/var["PLSTN"].value);
    }
    var["TEMFCF"].value = tabex(var, "GENTEM", "XTEM", var["TMPA"].value, 6);
    var["GENRF"].value = std::min(std::max(var["EPS"].value, var["CLSDML"].value)/var["GENFAC"].value,1.0) * var["TEMFCF"].value * tabex(var, "GENRAT", "XGEN", var["PLSTN"].value, 6);
    ANNA << var["CLSDML"].value << std::endl;
    var["RDVLVF"].value = tabex(var, "RDVLVT", "XLV", var["TMPA"].value, 9) * var["SPTEL"].value * var["FCO2"].value;
    var["RDVFRF"].value = tabex(var, "RDVFRT", "XFRT", var["TMPA"].value, 9) * var["SPTEL"].value * var["FCO2"].value;
    var["TTHF"].value = 0;
    var["TTLF"].value = 0;
    var["TTABF"].value = 0;
    if(var["TMPA"].value > var["THIGH"].value) var["TTHF"].value = var["TMPA"].value - var["THIGH"].value;
    if(var["TMPA"].value < var["TLOW"].value) var["TTLF"].value = var["TLOW"].value - var["TMPA"].value;
    if(var["TMPA"].value < var["TLOWAB"].value) var["TTABF"].value = var["TLOWAB"].value - var["TMPA"].value;
  }

  void Calc::photo(table& var){
    var["QE"].value = 0.056;
    var["XK"].value = 0.58;
    var["XM"].value = 0.10;
    var["GPF"].value = 0;
    var["TAU1"].value = 0.06638 * var["TU1"].value;
    var["TAU2"].value = 0.06638 * var["TU2"].value;
    var["PMAX"].value = var["TAU1"].value * var["CO2AVG"].value;
    if(var["CO2AVG"].value > 350.0) var["PMAX"].value = var["TAU1"].value * 350 + var["TAU2"].value * (var["CO2AVG"].value - 350);
    var["AEF"].value = tabex(var, "AEFT", "XAEFT", var["PLSTN"].value, 6);
    var["PMAX"].value = var["PMAX"].value * tabex(var, "PGRED", "TMPG", var["TMPA"].value, 8) * var["AEF"].value;

    if(var["PPFD"].value >= 0.001){
      var["TOP"].value = (1 - var["XM"].value) * var["PMAX"].value + var["QE"].value * var["XK"].value * var["PPFD"].value;
      var["BOT"].value = (1 - var["XM"].value) * var["PMAX"].value + var["QE"].value * var["XK"].value * var["PPFD"].value * std::exp(-1 * var["XK"].value * var["ASTOTL"].value * var["PLTM2V"].value);
      var["GPF"].value = (var["PMAX"].value / var["XK"].value) * std::log(var["TOP"].value / var["BOT"].value) * 0.682 * 3.8016;
    }
  }

  void Calc::resp(table& var){
    var["TEFF"].value = std::pow(var["Q10"].value, 0.1 * var["TMPA"].value - 2.0);
    var["MAINTF"].value = var["TEFF"].value * (var["RMRL"].value * (var["TOTWST"].value + var["WSTOTL"].value) + var["RMRF"].value * var["WTOTF"].value);
    //std::cout << var["TEFF"].value << " " << var["RMRL"].value << " " << var["TOTWST"].value << " " << var["WSTOTL"].value << " " << var["WTOTF"].value << std::endl;
  }

  void Calc::dmrate(table& var){
    //C-8
    var["PARSLA"].value = 1.0 - tabex(var, "PART", "XPART", var["PAR"].value, 5);
    var["ESLA"].value = var["STDSLA"].value * var["PARSLA"].value / (var["TSLA"].value * var["CSLA"].value);
    var["ESLA"].value = std::max(0.018, var["ESLA"].value);
    var["ESLA"].value = std::min(var["SLAMX"].value, var["ESLA"].value);
    var["TRCDRW"].value = (var["GP"].value / var["PLTM2V"].value - var["MAINT"].value) * var["GREF"].value;
    var["TRCDRW"].value = std::max(var["TRCDRW"].value, 0.0);
    var["RCDRW"].value = var["TRCDRW"].value * (1.0 - tabex(var, "PROOT", "XROOT", var["PLSTN"].value, 6.0)) * (std::min(std::max(var["EPS"].value, var["CLSDML"].value) / var["ZBENG"].value, 1.0) * var["TEMFAC"].value);
    //ANNA << var["TRCDRW"].value << "," << var["CLSDML"].value << "," << var["ZBENG"].value << std::endl;
    var["PTNLVS"].value = 0.0;
    var["PTNSTM"].value = 0.0;
    for(int i=0;i<var["NL"].value;i++){
      var["XBOX"].value = (i+1) * 100.0 / var["NL"].value;
      var[fileio->fixIndex("RCLFA", i)].value = var[fileio->fixIndex("LVSN", i)].value * tabex(var, "POL", "BOX", var["XBOX"].value, 10) * var["TEMFAC"].value * var["FCO2D"].value;
      var["FRPT"].value = tabex(var, "FRPET", "BOX", var["XBOX"].value, 10);
      var["FRST"].value = tabex(var, "FRSTEM", "BOX", var["XBOX"].value, 10);
      var[fileio->fixIndex("PNLVS", i)].value = (var[fileio->fixIndex("RCLFA", i)].value / (tabex(var, "ASLA", "BOX", var["XBOX"].value, 10) * var["ESLA"].value)) * (1.0 + var["FRPT"].value);
      var["PTNLVS"].value = var["PTNLVS"].value + var[fileio->fixIndex("PNLVS", i)].value;
      //ANNA << var[fileio->fixIndex("LVSN", i)].value << std::endl;
      var[fileio->fixIndex("PNSTM", i)].value = var[fileio->fixIndex("PNLVS", i)].value / (var[fileio->fixIndex("LVSN", i)].value + var["EPS"].value) * var["FRST"].value * var[fileio->fixIndex("STMS", i)].value;
      var["PTNSTM"].value = var["PTNSTM"].value + var[fileio->fixIndex("PNSTM", i)].value;
      //std::cout << "(*>△<)<" << var[fileio->fixIndex("PNLVS", i)].value << "," << var[fileio->fixIndex("RCLFA", i)].value << "," << var[fileio->fixIndex("LVSN", i)].value << "," << var["TEMFAC"].value << "," << var["FCO2D"].value << std::endl;
    }
    var["PTNFRT"].value = 0.0;
    for(int i=0;i<var["NF"].value;i++){
      //var[fileio->fixIndex("AVWF", i)].value = var[fileio->fixIndex("WFRT", i)].value / (var[fileio->fixIndex("FRTN", i)].value + var["EPS"].value); //仮
      var["ZZX"].value = std::min(1.0, std::max(var["EPS"].value, 2.0 - var[fileio->fixIndex("AVWF", i)].value / var["AVFM"].value));
      var["XBOX"].value = (i+1) * 100.0 / var["NF"].value;
      var[fileio->fixIndex("PNFRT", i)].value = var[fileio->fixIndex("FRTN", i)].value * tabex(var, "POF", "BOX", var["XBOX"].value, 10) * var["TEMFAC"].value * var["FCO2D"].value * var["ZZX"].value;
      var["PTNFRT"].value = var["PTNFRT"].value + var[fileio->fixIndex("PNFRT", i)].value;
    }  
    var["PNGP"].value = var["PTNLVS"].value + var["PTNFRT"].value + var["PTNSTM"].value;
    
    var["TOTDML"].value = std::min(var["RCDRW"].value * var["PTNLVS"].value / (var["PNGP"].value + var["EPS"].value), var["PTNLVS"].value);
    //ANNA << var["RCDRW"].value << "," << "," << var["PNGP"].value << "," << var["PTNLVS"].value << "," << std::endl;
    var["TOTDMS"].value = std::min(var["RCDRW"].value * var["PTNSTM"].value / (var["PNGP"].value + var["EPS"].value), var["PTNSTM"].value);
    var["TOTDMF"].value = std::min(var["RCDRW"].value * var["PTNFRT"].value / (var["PNGP"].value + var["EPS"].value), var["PTNFRT"].value);
    var["TOPGR"].value = var["TOTDMF"].value + var["TOTDML"].value + var["TOTDMS"].value;
    var["EXCESS"].value = var["RCDRW"].value - var["TOPGR"].value;
    var["CLSDMF"].value = 1.0;
    if(var["PTNFRT"].value > 0.0) var["CLSDMF"].value = var["TOTDMF"].value / (var["PTNFRT"].value + var["EPS"].value);
    var["CLSDML"].value = var["TOTDML"].value / (var["PTNLVS"].value + var["EPS"].value);
    //ANNA << var["TOTDML"].value << "," << var["PTNLVS"].value << std::endl;
    for(int i=0;i<var["NL"].value;i++){
      var[fileio->fixIndex("RCWLV", i)].value = var["TOTDML"].value * var[fileio->fixIndex("PNLVS", i)].value / (var["PTNLVS"].value + var["EPS"].value);
      var[fileio->fixIndex("RCWST", i)].value = var["TOTDMS"].value * var[fileio->fixIndex("PNSTM", i)].value / (var["PTNSTM"].value + var["EPS"].value);
      var["XBOX"].value = (i+1) * 100.0 / var["NL"].value;
      var["FRPT"].value = tabex(var, "FRPET", "BOX", var["XBOX"].value, 10);
      var[fileio->fixIndex("RCLFA", i)].value = var[fileio->fixIndex("RCWLV", i)].value * tabex(var, "ASLA", "BOX", var["XBOX"].value, 10) * var["ESLA"].value / (1.0 + var["FRPT"].value);
    }
    for(int i=0;i<var["NF"].value;i++){
      var[fileio->fixIndex("RCWFR", i)].value = var["TOTDMF"].value * var[fileio->fixIndex("PNFRT", i)].value / (var["PTNFRT"].value + var["EPS"].value);
    }
  }

  void Calc::devrate(table& var){
    var["TPLA"].value = 0.0;
    if(var["PLSTN"].value > var["FTRUSN"].value) var["TPLA"].value = var["TPL"].value;
    var["RCNL"].value = var["PLM2"].value * var["GENR"].value / (1.0 + var["TPLA"].value);
    var["RCST"].value = var["PLM2"].value * var["GENR"].value;
    var["RCNF"].value = var["GENR"].value * tabex(var, "FPN", "XFPN", var["PLSTN"].value - var["FRLG"].value, 10) * var["PLM2"].value;
    var["RCNF"].value = var["RCNF"].value * std::max(1.0 - var["TTH"].value / var["TTMX"].value, 0.0) * std::max(1.0 + var["TTL"].value / var["TTMN"].value, 0.0);
    var["PUSHL"].value = var["RDVLV"].value * var["NL"].value;
    var["PUSHM"].value = var["RDVFR"].value * var["NF"].value;
  }

  void Calc::losrate(table& var){
    var["ABNF"].value = 0.0;
    if(var["TOTDMF"].value >= var["EPS"].value){
      var["FABOR"].value = std::min(1.0, 2.0 - var["ABORMX"].value * var["CLSDML"].value);
      var["FABOR"].value = std::max(0.0, var["FABOR"].value);
      var["TABOR"].value = std::min(1.0, std::max(0.0, var["TTAB"].value/var["TABK"].value));
      var["ABNF"].value = var["FABOR"].value * var["RCNF"].value / var["PLTM2V"].value + var["TABOR"].value * var["RCNF"].value / var["PLTM2V"].value;
    }
    var[fileio->fixIndex("DEAR", var["NL"].value)].value = 0.0;
    if(var["XLAI"].value * var["PLTM2V"].value > var["XLAIM"].value)  /*誤字？*/ var[fileio->fixIndex("DEAR", var["NL"].value)].value = var["XMRDR"].value * std::min(var[fileio->fixIndex("LFAR", var["NL"].value)].value, (var["XLAI"].value * var["PLTM2V"].value - var["XLAIM"].value) / var["PLTM2V"].value);
    var[fileio->fixIndex("DEAR", var["NL"].value)].value = std::max(0.0, var[fileio->fixIndex("DEAR", var["NL"].value)].value);
    var["DATEZ"].value = tabex(var, "DISDAT", "XDISDAT", var["TIME"].value, 12);

    for(int i=0;i<var["NL"].value;i++){ //誤字？ var["NL"].value - 1
      var["XBOX"].value = (i+1) * 100.0 / var["NL"].value;
      var[fileio->fixIndex("DEAR", i)].value = tabex(var, "DIS", "BOX", var["XBOX"].value, 10) * var["DATEZ"].value;
    }

    for(int i=0;i<var["NL"].value;i++){
      var[fileio->fixIndex("AVWL", i)].value = var[fileio->fixIndex("WLVS", i)].value / (var[fileio->fixIndex("LVSN", i)].value + var["EPS"].value);
      var[fileio->fixIndex("DENLR", i)].value = var[fileio->fixIndex("LVSN", i)].value * var[fileio->fixIndex("DEAR", i)].value;
      var[fileio->fixIndex("DEWLR", i)].value = var[fileio->fixIndex("DENLR", i)].value * var[fileio->fixIndex("AVWL", i)].value;
      var[fileio->fixIndex("DELAR", i)].value = var[fileio->fixIndex("DEAR", i)].value * var[fileio->fixIndex("LFAR", i)].value;
    }

    for(int i=0;i<var["NF"].value;i++){
      var["XBOX"].value = (i+1) * 100.0 / var["NF"].value;
      var[fileio->fixIndex("DEAF", i)].value = tabex(var, "DISF", "BOX", var["XBOX"].value, 10) * var["DATEZ"].value;
      var[fileio->fixIndex("DENFR", i)].value = var[fileio -> fixIndex("FRTN", i)].value * var[fileio -> fixIndex("DEAF", i)].value;
      var[fileio->fixIndex("DEWFR", i)].value = var[fileio -> fixIndex("DENFR", i)].value * var[fileio -> fixIndex("AVWF", i)].value;
    }
  }

  void Calc::intgrat(table& var){
    //C-16
    var["CPOOL"].value = var["CPOOL"].value + (var["GP"].value - var["RCDRW"].value / var["GREF"].value - var["MAINT"].value) * var["DELT"].value;
    var["PLSTN"].value = var["PLSTN"].value + var["GENR"].value * var["DELT"].value;
    var[fileio->fixIndex("LVSN", var["NL"].value)].value = var[fileio->fixIndex("LVSN", var["NL"].value)].value + (var["PUSHL"].value * var[fileio->fixIndex("LVSN", var["NL"].value-1)].value - var[fileio->fixIndex("DENLW", var["NL"].value)].value) * var["DELT"].value;
    var[fileio->fixIndex("WLVS", var["NL"].value)].value = var[fileio->fixIndex("WLVS", var["NL"].value)].value + (var["PUSHL"].value * var[fileio->fixIndex("WLVS", var["NL"].value-1)].value - var[fileio->fixIndex("DEWLR", var["NL"].value)].value) * var["DELT"].value;
    var[fileio->fixIndex("LFAR", var["NL"].value)].value = var[fileio->fixIndex("LFAR", var["NL"].value)].value + (var["PUSHL"].value * var[fileio->fixIndex("LFAR", var["NL"].value-1)].value - var[fileio->fixIndex("DELAR", var["NL"].value)].value) * var["DELT"].value;
    var[fileio->fixIndex("STMS", var["NL"].value)].value = var[fileio->fixIndex("STMS", var["NL"].value)].value + var["PUSHL"].value * var[fileio->fixIndex("STMS", var["NL"].value-1)].value * var["DELT"].value;
    var[fileio->fixIndex("WSTM", var["NL"].value)].value = var[fileio->fixIndex("WSTM", var["NL"].value)].value + var["PUSHL"].value * var[fileio->fixIndex("WSTM", var["NL"].value-1)].value * var["DELT"].value;

    for(int i=1;i<var["NL"].value;i++){
      int ii = var["NL"].value - i;
      //ANNA << var[fileio->fixIndex("LVSN", ii)].value << std::endl;
      var[fileio->fixIndex("LVSN", ii)].value = var[fileio->fixIndex("LVSN", ii)].value + var["PUSHL"].value * (var[fileio->fixIndex("LVSN", ii-1)].value - var[fileio->fixIndex("LVSN", ii)].value) * var["DELT"].value - var[fileio->fixIndex("DENLR", ii)].value * var["DELT"].value;
      //ANNA << var["PUSHL"].value * (var[fileio->fixIndex("LVSN", ii-1)].value - var[fileio->fixIndex("LVSN", ii)].value) * var["DELT"].value - var[fileio->fixIndex("DENLR", ii)].value * var["DELT"].value << std::endl;
      var[fileio->fixIndex("STMS", ii)].value = var[fileio->fixIndex("STMS", ii)].value + var["PUSHL"].value * (var[fileio->fixIndex("STMS", ii-1)].value - var[fileio->fixIndex("STMS", ii)].value) * var["DELT"].value;
      var[fileio->fixIndex("WLVS", ii)].value = var[fileio->fixIndex("WLVS", ii)].value + var["PUSHL"].value * (var[fileio->fixIndex("WLVS", ii-1)].value - var[fileio->fixIndex("WLVS", ii)].value) + var[fileio->fixIndex("RCWLV", ii)].value * var["DELT"].value - var[fileio->fixIndex("DEWLR", ii)].value * var["DELT"].value;
      var[fileio->fixIndex("WSTM", ii)].value = var[fileio->fixIndex("WSTM", ii)].value + (var["PUSHL"].value * (var[fileio->fixIndex("WSTM", ii-1)].value - var[fileio->fixIndex("WSTM", ii)].value) + var[fileio->fixIndex("RCWST", ii)].value) * var["DELT"].value;
      var[fileio->fixIndex("LFAR", ii)].value = var[fileio->fixIndex("LFAR", ii)].value + (var["PUSHL"].value * (var[fileio->fixIndex("LFAR", ii-1)].value - var[fileio->fixIndex("LFAR", ii)].value) + var[fileio->fixIndex("RCWST", ii)].value) * var["DELT"].value - var[fileio->fixIndex("DELAR", ii)].value * var["DELT"].value;
    }

    var["LVSN(0)"].value = (var["RCNL"].value - var["PUSHL"].value * var["LVSN(0)"].value) * var["DELT"].value - var["DENLR(0)"].value * var["DELT"].value;
    //ANNA << var["RCNL"].value << "," << var["PUSHL"].value << "," << var["LVSN(0)"].value << "," << var["DENLR(0)"].value << std::endl;
    var["STMS(0)"].value = var["STMS(0)"].value + (var["RCST"].value - var["PUSHL"].value * var["STMS(0)"].value) * var["DELT"].value;
    var["WLVS(0)"].value = (var["RCNL"].value * var["WPLI"].value - var["PUSHL"].value * var["WLVS(0)"].value + var["RCWLV(0)"].value) * var["DELT"].value + var["WLVS(0)"].value - var["DEWLR(0)"].value * var["DELT"].value;
    var["WSTM(0)"].value = var["WSTM(0)"].value + (var["RCST"].value * var["WPLI"].value * var["FRSTEM(0)"].value - var["PUSHL"].value * var["WSTM(0)"].value + var["RCWST(0)"].value) * var["DELT"].value;
    var["FRPT"].value = 1.0 + var["FRPET(0)"].value;
    var["LFAR(0)"].value = (var["RCNL"].value * var["WPLI"].value * var["ESLA"].value * var["ASLA(0)"].value / var["FRPT"].value - var["PUSHL"].value * var["LFAR(0)"].value + var["RCLFA(0)"].value) * var["DELT"].value + var["LFAR(0)"].value - var["DELAR(0)"].value * var["DELT"].value;

    var[fileio->fixIndex("FRTN", var["NF"].value)].value = var[fileio->fixIndex("FRTN", var["NF"].value)].value + (var["PUSHM"].value * var[fileio->fixIndex("FRTN", var["NF"].value-1)].value - var[fileio->fixIndex("DENFR", var["NF"].value)].value) * var["DELT"].value;
    var[fileio->fixIndex("WFRT", var["NF"].value)].value = var[fileio->fixIndex("WFRT", var["NF"].value)].value + (var["PUSHM"].value * var[fileio->fixIndex("WFRT", var["NF"].value-1)].value - var[fileio->fixIndex("DEWFR", var["NF"].value)].value) * var["DELT"].value;

    for(int i=1;i<var["NF"].value-1;i++){
      int ii = var["NF"].value - i + 1;
      var[fileio->fixIndex("FRTN", ii)].value = var[fileio->fixIndex("FRTN", ii)].value + var["PUSHM"].value * (var[fileio->fixIndex("FRTN", ii-1)].value - var[fileio->fixIndex("FRTN", ii)].value) * var["DELT"].value - var[fileio->fixIndex("DENFR", ii)].value * var["DELT"].value;
      var[fileio->fixIndex("WFRT", ii)].value = var[fileio->fixIndex("WFRT", ii)].value + (var["PUSHM"].value * (var[fileio->fixIndex("WFRT", ii-1)].value - var[fileio->fixIndex("WFRT", ii)].value) + var[fileio->fixIndex("RCWFR", ii)].value) * var["DELT"].value - var[fileio->fixIndex("DEWFR", ii)].value * var["DELT"].value;
    }
    var["FRTN(0)"].value = (var["RCNF"].value - var["ABNF"].value - var["PUSHM"].value * var["FRTN(0)"].value) * var["DELT"].value + var["FRTN(0)"].value - var["DENFR(0)"].value * var["DELT"].value;
    var["WFRT(0)"].value = ((var["RCNF"].value - var["ABNF"].value) * var["WPFI"].value - var["PUSHM"].value * var["WFRT(0)"].value + var["RCWFR(0)"].value) * var["DELT"].value + var["WFRT(0)"].value - var["DEWFR(0)"].value * var["DELT"].value;
    
    var["XLAI"].value = 0.0;
    var["TWTLAI"].value = 0.0;
    var["TOTNLV"].value = 0.0;
    var["TOTWML"].value = 0.0;
    var["TOTNST"].value = 0.0;
    var["TOTWST"].value = 0.0;
    var["ATV"].value = 0.0;

    for(int i=0;i<var["NL"].value;i++){
      var[fileio->fixIndex("AVWL", i)].value = var[fileio->fixIndex("WLVS", i)].value / (var[fileio->fixIndex("LVSN", i)].value + var["EPS"].value);
      var["XLAI"].value = var["XLAI"].value + var[fileio->fixIndex("LFAR", i)].value;
      var["TOTNLV"].value = var["TOTNLV"].value + var[fileio->fixIndex("LVSN", i)].value;
      var["TOTWML"].value = var["TOTWML"].value + var[fileio->fixIndex("WLVS", i)].value;
      var["ATL"].value = var["ATL"].value + var[fileio->fixIndex("DEWLR", i)].value * var["DELT"].value;
      var["XBOX"].value = (i+1) * 100 / var["NL"].value;
      var["FRPT"].value = tabex(var, "FRPET", "BOX", var["XBOX"].value, 10);
      var["TWTLAI"].value = var["TWTLAI"].value + var[fileio->fixIndex("WLVS", i)].value / (1.0 + var["FRPT"].value);
      var["TOTNST"].value = var["TOTNST"].value + var[fileio->fixIndex("STMS", i)].value;
      var["TOTWST"].value = var["TOTWST"].value + var[fileio->fixIndex("WSTM", i)].value;
    }

    var["XSLA"].value = var["XLAI"].value / (var["TWTLAI"].value + var["EPS"].value) * 10000.0;
    var["TOTWMF"].value = 0.0;
    var["TOTNF"].value = 0.0;

    for(int i=0;i<var["NF"].value;i++){
      var[fileio->fixIndex("AVWF", i)].value = var[fileio->fixIndex("WFRT", i)].value / (var[fileio->fixIndex("FRTN", i)].value + var["EPS"].value);
      var["TOTWMF"].value = var["TOTWMF"].value + var[fileio->fixIndex("WFRT", i)].value;
      var["TOTNF"].value = var["TOTNF"].value + var[fileio->fixIndex("FRTN", i)].value;
    }

    var["WTOTF"].value = var["TOTWMF"].value - var[fileio->fixIndex("WFRT", var["NF"].value)].value;
    var["TOTGF"].value = var["TOTNF"].value - var[fileio->fixIndex("FRTN", var["NF"].value)].value; //1e10
    var["BTOTNLV"].value = var["BTOTNLV"].value + var["RCNL"].value * var["DELT"].value;
    var["DLN"].value = (var["BTOTNLV"].value - var["TOTNLV"].value) / var["PLM2"].value;
    var["TOTGL"].value = 0.0;
    var["ASTOTL"].value = 0.0;
    var["WSTOTL"].value = var["TOTWML"].value - var[fileio->fixIndex("WLVS", var["NL"].value)].value;
    var["TOTGL"].value = var["TOTNLV"].value - var[fileio->fixIndex("LVSN", var["NL"].value)].value;
    var["ASTOTL"].value = var["XLAI"].value - var[fileio->fixIndex("LFAR", var["NL"].value)].value;
    var["TOTST"].value = var["TOTNST"].value - var[fileio->fixIndex("STMS", var["NL"].value)].value;
    var["WSTOTS"].value = var["TOTWST"].value - var[fileio->fixIndex("WSTM", var["NL"].value)].value;
    var["TOTDW"].value = var["TOTWMF"].value + var["TOTWML"].value + var["TOTWST"].value;
    var["TOTVM"].value = var["TOTWML"].value + var["TOTWST"].value;
    var["ATV"].value = var["TOTWML"].value + var["TOTWST"].value + var["ATL"].value;
    var["ATT"].value = var["ATV"].value + var["TOTWMF"].value;
    var["TOTNU"].value = var["TOTNF"].value + var["TOTNLV"].value;
    var["NGP"].value = var["TOTGL"].value + var["TOTGF"].value + var["TOTST"].value; 
    var["RVRW"].value = var["TOTWMF"].value / (var["TOTWML"].value + var["EPS"].value);
    var["RRRW"].value = var["TOTWMF"].value / (var["TOTDW"].value + var["EPS"].value);
    var["RVRN"].value = var["TOTNF"].value / (var["TOTNLV"].value + var["EPS"].value);
    var["RTRN"].value = var["TOTNF"].value / (var["TOTNU"].value + var["EPS"].value);
    var["AVWMF"].value = var["TOTWMF"].value / (var["TOTNF"].value + var["EPS"].value);
    var["AVWML"].value = var["TOTWML"].value / (var["TOTNLV"].value + var["EPS"].value);
    var["DMCF84"].value = tabex(var, "DMC84T", "XDMC", var["TIME"].value, 6);
    var["FWFR10"].value = var["FWFR10"].value + (var["PUSHM"].value * var[fileio->fixIndex("WFRT", var["NF"].value-1)].value * var["DELT"].value) * 100.0 / var["DMCF84"].value;
    var["APFFW"].value = ((var["PUSHM"].value * std::min(var[fileio->fixIndex("WFRT", var["NF"].value-1)].value, 0.0) * var["DELT"].value) * 100.0 / var["DMCF84"].value) / ((var["PUSHM"].value * var[fileio->fixIndex("FRTN", var["NF"].value-1)].value * var["DELT"].value) + var["EPS"].value);
  }
}