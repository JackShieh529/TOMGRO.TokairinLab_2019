#include <string>
#include <unordered_map>

namespace tomgro{
class FileIO{
  public:
    void test();
    std::unordered_map<std::string, double> data;
    std::unordered_map<std::string, double> inputData(std::string fileName);
    std::unordered_map<std::string, double> initialCondition(std::string fileName);
    std::unordered_map<std::string, double> inputWeather(std::string fileName);
};
}