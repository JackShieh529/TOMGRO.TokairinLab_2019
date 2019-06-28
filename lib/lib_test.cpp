#include "lib.hpp"
#include <map>

int main(){
  tomgro::FileIO io; //io:File Input and Output
  tomgro::table<double> data; //variables, parameters
  io.inputData(data, "../src/MGT.csv"); //get params from MGT.csv
  io.inputData(data, "../src/CropParam.csv");
  io.initializeVariables(data); //calc initial values
  io.inputWeather(data, "../src/Weather.csv");
  
  for(int jday=0;jday<data["NDAYS"];jday++){
    io.change(data, "PLTM2V", 22);
    if(jday > 100) io.change(data, "PLTM2V", 3.0);
    else if(jday > 84) io.change(data, "PLTM2V", 4.0);
    else if(jday > 66) io.change(data, "PLTM2V", 5.0);
    else if(jday > 57) io.change(data, "PLTM2V", 6.0);
    else if(jday > 45) io.change(data, "PLTM2V", 6.5);
    else if(jday > 36) io.change(data, "PLTM2V", 7.0);
    else if(jday > 21) io.change(data, "PLTM2V", 8.5);
    else if(jday > 17) io.change(data, "PLTM2V", 11.0);
    else if(jday > 10) io.change(data, "PLTM2V", 15.5);
  }

  io.change(data, "TIME", (data["JDAY"]-1)*data["DELT"]);
  io.change(data, "START", data["NSTART"]);
  io.change(data, "DATE", ((data["TIME"]+data["START"])%365)+1);

  
  return 0;
}