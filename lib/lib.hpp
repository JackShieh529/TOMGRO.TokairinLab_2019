#include <string>
#include <unordered_map>
#include <fstream>
#include <vector>

namespace tomgro{
template <class T>
using table = std::unordered_map<std::string, T>;

class FileIO{
  public:
    void test();
    table<std::string, double> data;
    table<std::string, double> inputData(std::string fileName);
    table<std::string, double> initialCondition(std::string fileName);
    table<std::string, double> inputWeather(std::string fileName);
};
}
