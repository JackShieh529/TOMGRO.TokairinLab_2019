/*****
1. 変数名打ち間違い
2. minとmaxの打ち間違い
3. カウンター変数(1 -> 0 にしてる)による誤計算
*****/

#include "lib.hpp"
#include <map>

#define ANNA std::cout << "(*>△<)<"

int main(){
  tomgro::FileIO io; //io:File Input and Output
  tomgro::Calc cl; //cl:Calc Calculation
  tomgro::table var; //variables, parameters

  std::ifstream ifs("../src/Weather.csv");
  std::vector<std::string> pair;
  std::string header[7] = {"KYR", "JUL", "SOLRAD", "TMAX", "TMIN", "RAIN", "PARO"};

  var["IRUNNUM"].value = 0;
  for(int i=0;i<24;i++) var[io.fixIndex("HOURS", i)].value = i;

  io.inputData(var, "../src/MGT.csv"); //get params from MGT.csv
  io.inputData(var, "../src/CropParam.csv");
  io.initializeVariables(var); //calc initial values

  if(!ifs.is_open()){
    std::cout << "Can't open file" << std::endl;
    return 0;
  }

  var["NEW"].value = 0; //0:First work
  
  //First data
  var["XLAT"].value = 31.2;
  var["XLONG"].value = 34.4;
  var["PARFAC"].value = 12.07;
  var["PARDAT"].value = 0.5;

  //Start read waeather data
  var["JDAY"].value = 0;
  while (!ifs.eof()){
    try{
      std::string buffer;
      ifs >> buffer;
      if(buffer == "" || buffer == "\n" || buffer == "\t" ) continue;
      pair = io.split(buffer, ',');
      //For waeather file
      for(int col=0;col<pair.size();col++){
        var[header[col]].value = std::stod(pair[col]);
      }
    }catch(std::exception e_range){
      continue;
    }

    //Reading file starts from NSTART day
    if(var["NEW"].value == 0){
      if(var["JUL"].value > var["NSTART"].value){
        std::cout << "The data is not correct." << std::endl;
        break;
      }else if(var["JUL"].value < var["NSTART"].value){
        continue;
      }else{
        var["NEW"].value = 1;
      }
    }

    var["XLANG"].value = var["SOLRAD"].value * 23.923;
    cl.sunrise(var, "JUL", "SUP", "SDN", "XLAT", "XLONG");
    if(var["PARDAT"].value < 0) var["PARO"].value = var["XLANG"].value / var["PARFAC"].value;

    io.change(var, "PLTM2V", 22);
    if(var["JDAY"].value > 100) io.change(var, "PLTM2V", 3.0);
    else if(var["JDAY"].value > 84) io.change(var, "PLTM2V", 4.0);
    else if(var["JDAY"].value > 66) io.change(var, "PLTM2V", 5.0);
    else if(var["JDAY"].value > 57) io.change(var, "PLTM2V", 6.0);
    else if(var["JDAY"].value > 45) io.change(var, "PLTM2V", 6.5);
    else if(var["JDAY"].value > 36) io.change(var, "PLTM2V", 7.0);
    else if(var["JDAY"].value > 21) io.change(var, "PLTM2V", 8.5);
    else if(var["JDAY"].value > 17) io.change(var, "PLTM2V", 11.0);
    else if(var["JDAY"].value > 10) io.change(var, "PLTM2V", 15.5);

    io.change(var, "TIME", (var["JDAY"].value-1)*var["DELT"].value);
    io.change(var, "START", var["NSTART"].value);
    io.change(var, "DATE", (fmod(var["TIME"].value+var["START"].value, 365))+1);

    cl.calcWeather(var);

    var["GP"].value = 0;
    var["MAINT"].value = 0;
    var["GENR"].value = 0;
    var["TEMFAC"].value = 0;
    var["RDVLV"].value = 0;
    var["RDVFR"].value = 0;
    var["FCO2D"].value = 0;
    var["NCSLA"].value = 0;
    var["TSLA"].value = 0;
    var["CSLA"].value = 0;
    var["TTH"].value = 0;
    var["TTL"].value = 0;
    var["TTAB"].value = 0;

    for(int jf=0;jf<var["NFAST"].value;jf++){
      var["TFAST"].value = jf * 24.0 / var["NFAST"].value;
      cl.ghouse(var);
      /* ####################### 
      std::cout << "e---nan" << var["JDAY"].value << "nan---e" << var["JUL"].value << std::endl;
      for(auto p : var) std::cout << p.first << ":" << p.second.value << std::endl;
      /* ####################### */
      cl.devfast(var);
      cl.photo(var);
      cl.resp(var);

      var["GPFN"].value = (var["GPF"].value - var["MAINTF"].value) * var["GREF"].value;
      var["GENR"].value = var["GENR"].value + var["GENRF"].value * var["DTFAST"].value;
      //ANNA << var["GENRF"].value << "," << var["DTFAST"].value << std::endl;
      var["TEMFAC"].value = var["TEMFAC"].value + var["TEMFCF"].value * var["DTFAST"].value;
      var["RDVLV"].value = var["RDVLV"].value + var["RDVLVF"].value * var["DTFAST"].value;
      var["RDVFR"].value = var["RDVFR"].value + var["RDVFRF"].value * var["DTFAST"].value;
      var["TTH"].value = var["TTH"].value + var["TTHF"].value * var["DTFAST"].value;
      var["TTL"].value = var["TTL"].value + var["TTLF"].value * var["DTFAST"].value;
      var["TTAB"].value = var["TTAB"].value + var["TTABF"].value * var["DTFAST"].value;
      var["FCO2D"].value = var["FCO2D"].value + var["FCO2"].value * var["DTFAST"].value;
      var["TSLA"].value = var["TSLA"].value + var["TSLAF"].value * var["DTFAST"].value;
      var["CSLA"].value = var["CSLA"].value + var["CSLAF"].value;
      var["GP"].value = var["GP"].value + var["GPF"].value * var["DTFAST"].value;
      var["MAINT"].value = var["MAINT"].value + var["MAINTF"].value * var["DTFAST"].value;
    }

    if(var["NCSLA"].value == 0) var["CSLA"].value = 1.0;
    else var["CSLA"].value = var["CSLA"].value / var["NCSLA"].value;

    var["TSLA"].value = std::max(var["TSLA"].value, 0.1);

    cl.dmrate(var);
    cl.devrate(var);
    cl.losrate(var);

    double itrtnum = 0.0;
    while(var["JDAY"].value == 0 && itrtnum < 1 || itrtnum > 20){
      //std::cin >> itrtnum;
      itrtnum = 1;
      var["ITRTNUM"].value = itrtnum;
    }

    //std::cout << "(*>△<)<" << var["TOTNL/PLM2"].value << std::endl;

    std::cout << "Simulation continues ... Please wait." << std::endl;

    cl.intgrat(var);
    //std::map<std::string, double> normal_map(var.begin(),var.end());
    

    io.output(var);
    var["JDAY"].value = var["JDAY"].value + 1;
  }

  /*
  for(int jday=0;jday<var["NDAYS"].value;jday++){
    std::cout << jday << "---nannan---" << std::endl;
    var["JDAY"].value = jday;
    io.change(var, "PLTM2V", 22);
    if(jday > 100) io.change(var, "PLTM2V", 3.0);
    else if(jday > 84) io.change(var, "PLTM2V", 4.0);
    else if(jday > 66) io.change(var, "PLTM2V", 5.0);
    else if(jday > 57) io.change(var, "PLTM2V", 6.0);
    else if(jday > 45) io.change(var, "PLTM2V", 6.5);
    else if(jday > 36) io.change(var, "PLTM2V", 7.0);
    else if(jday > 21) io.change(var, "PLTM2V", 8.5);
    else if(jday > 17) io.change(var, "PLTM2V", 11.0);
    else if(jday > 10) io.change(var, "PLTM2V", 15.5);

    io.change(var, "TIME", (var["JDAY"].value-1)*var["DELT"].value);
    io.change(var, "START", var["NSTART"].value);
    io.change(var, "DATE", (fmod(var["TIME"].value+var["START"].value, 365))+1);

    io.inputWeather(var, "../src/Weather.csv"); //set weather var

    cl.calcWeather(var);

    var["GP"].value = 0;
    var["MAINT"].value = 0;
    var["GENR"].value = 0;
    var["TEMFAC"].value = 0;
    var["RDVLV"].value = 0;
    var["RDVFR"].value = 0;
    var["FCO2D"].value = 0;
    var["NCSLA"].value = 0;
    var["TSLA"].value = 0;
    var["CSLA"].value = 0;
    var["TTH"].value = 0;
    var["TTL"].value = 0;
    var["TTAB"].value = 0;

    cl.ghouse(var);
    cl.devfast(var);
    cl.photo(var);
    cl.resp(var);

    var["GPFN"].value = (var["GPF"].value - var["MAINTF"].value) * var["GREF"].value;
    var["GENR"].value = var["GENR"].value + var["GENRF"].value * var["DTFAST"].value;
    var["TEMFAC"].value = var["TEMFAC"].value + var["TEMFCF"].value * var["DTFAST"].value;
    var["RDVLV"].value = var["RDVLV"].value + var["RDVLVF"].value * var["DTFAST"].value;
    var["RDVFR"].value = var["RDVFR"].value + var["RDVLRF"].value * var["DTFAST"].value;
    var["TTH"].value = var["TTH"].value + var["TTHF"].value * var["DTFAST"].value;
    var["TTL"].value = var["TTL"].value + var["TTLF"].value * var["DTFAST"].value;
    var["TTAB"].value = var["TTAB"].value + var["TTABF"].value * var["DTFAST"].value;
    var["FCO2D"].value = var["FCO2D"].value + var["FCO2"].value * var["DTFAST"].value;
    var["TSLA"].value = var["TSLA"].value + var["TSLAF"].value * var["DTFAST"].value;
    var["CSLA"].value = var["CSLA"].value + var["CSLAF"].value;
    var["GP"].value = var["GP"].value + var["GPF"].value * var["DTFAST"].value;
    var["MAINT"].value = var["MAINT"].value + var["MAINTF"].value * var["DTFAST"].value;

    if(var["NCSLA"].value == 0) var["CSLA"].value = 1.0;
    else var["CSLA"].value = var["CSLA"].value / var["NCSLA"].value;

    var["TSLA"].value = std::max(var["TSLA"].value, 0.1);

    cl.dmrate(var);
    cl.devrate(var);
    cl.losrate(var);

    double itrtnum = 0.0;
    while(var["JDAY"].value == 0 && itrtnum < 1 || itrtnum > 20){
      //std::cin >> itrtnum;
      itrtnum = 1;
      var["ITRTNUM"].value = itrtnum;
    }

    std::cout << "Simulation continues ... Please wait." << std::endl;

    cl.intgrat(var);
    std::map<std::string, double> normal_map(var.begin(),var.end());
    //for(auto p : normal_map) std::cout << p.first << ":" << p.second << std::endl;

    io.output(var);
    std::cout << var["TOTNL/PLM2"].value << std::endl;
  }
  */

  return 0;
}