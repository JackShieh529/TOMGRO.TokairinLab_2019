/*****
1. 変数名打ち間違い
2. minとmaxの打ち間違い
3. カウンター変数(1 -> 0 にしてる)による誤計算
*****/

#include "lib.hpp"
#include <map>

int main(){
  tomgro::FileIO io; //io:File Input and Output
  tomgro::Calc cl; //cl:Calc Calculation
  tomgro::table<double> var; //variables, parameters

  var["IRUNNUM"] = 0;
  for(int i=0;i<24;i++) var[io.fixIndex("HOURS", i)] = i;

  io.inputData(var, "../src/MGT.csv"); //get params from MGT.csv
  io.inputData(var, "../src/CropParam.csv");
  io.initializeVariables(var); //calc initial values
  
  for(int jday=0;jday<var["NDAYS"];jday++){
    var["JDAY"] = jday;
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

    io.change(var, "TIME", (var["JDAY"]-1)*var["DELT"]);
    io.change(var, "START", var["NSTART"]);
    io.change(var, "DATE", (fmod(var["TIME"]+var["START"], 365))+1);

    io.inputWeather(var, "../src/Weather.csv"); //set weather var
    cl.calcWeather(var);

    var["GP"] = 0;
    var["MAINT"] = 0;
    var["GENR"] = 0;
    var["TEMFAC"] = 0;
    var["RDVLV"] = 0;
    var["RDVFR"] = 0;
    var["FCO2D"] = 0;
    var["NCSLA"] = 0;
    var["TSLA"] = 0;
    var["CSLA"] = 0;
    var["TTH"] = 0;
    var["TTL"] = 0;
    var["TTAB"] = 0;

    cl.ghouse(var);
    cl.devfast(var);
    cl.photo(var);
    cl.resp(var);

    var["GPFN"] = (var["GPF"] - var["MAINTF"]) * var["GREF"];
    var["GENR"] = var["GENR"] + var["GENRF"] * var["DTFAST"];
    var["TEMFAC"] = var["TEMFAC"] + var["TEMFCF"] * var["DTFAST"];
    var["RDVLV"] = var["RDVLV"] + var["RDVLVF"] * var["DTFAST"];
    var["RDVFR"] = var["RDVFR"] + var["RDVLRF"] * var["DTFAST"];
    var["TTH"] = var["TTH"] + var["TTHF"] * var["DTFAST"];
    var["TTL"] = var["TTL"] + var["TTLF"] * var["DTFAST"];
    var["TTAB"] = var["TTAB"] + var["TTABF"] * var["DTFAST"];
    var["FCO2D"] = var["FCO2D"] + var["FCO2"] * var["DTFAST"];
    var["TSLA"] = var["TSLA"] + var["TSLAF"] * var["DTFAST"];
    var["CSLA"] = var["CSLA"] + var["CSLAF"];
    var["GP"] = var["GP"] + var["GPF"] * var["DTFAST"];
    var["MAINT"] = var["MAINT"] + var["MAINTF"] * var["DTFAST"];

    if(var["NCSLA"] == 0) var["CSLA"] = 1.0;
    else var["CSLA"] = var["CSLA"] / var["NCSLA"];

    var["TSLA"] = std::max(var["TSLA"], 0.1);

    cl.dmrate(var);
    cl.devrate(var);
    cl.losrate(var);

    double itrtnum = 0.0;
    while(var["JDAY"] == 0 && itrtnum < 1 || itrtnum > 20){
      std::cin >> itrtnum;
      var["ITRTNUM"] = itrtnum;
    }

    std::cout << "Simulation continues ... Please wait." << std::endl;

    

    //std::cout << var["MAINTF"] << std::endl;
  }

  return 0;
}