#include <cassert>
#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <iostream>
#include <sstream>
#define ENABLE_YAML 1
#if ENABLE_YAML
#include <yaml-cpp/yaml.h>
#endif // ENABLE_YAML

#include "hiker.h"
#include "bridge.h"
#include "cache.h"
#include "calculator.h"
#include "utils.h"

using std::string;
using std::vector;
using std::map;


#if ENABLE_YAML
// YAML example:
// hikers:
// - name: A
//   speed: 100
// - name: B
//   speed: 50
// bridges:
// - length: 100
// - length: 250
//   hikers:
//   - name: E
//     speed: 2.5
// - length: 150
//   hikers:
//   - name: F
//     speed: 25
//   - name: G
//     speed: 15

class YAMLCaseParser {
public:
    void parse(const YAML::Node& node, vector<Hiker>& origHikers, vector<Bridge>& bridges) {
        parseHikers(node["hikers"], origHikers);
        sort_hikers(origHikers);
        vector<Hiker> additionalHikers;
        for (auto& bridge : node["bridges"]) {
            double length = 0.0;
            size_t size = additionalHikers.size();
            parseBridge(bridge, length, additionalHikers);
            // New additional hiker(s) are added, need sort.
            if (additionalHikers.size() > size) {
                sort_hikers(additionalHikers);
            }
            bridges.emplace_back(Bridge(length, additionalHikers));
        }
    }

protected:
    Hiker parseHiker(const YAML::Node& node) {
        string name = node["name"].as<string>();
        double speed = node["speed"].as<double>();
        if (speed > 0) {
            return Hiker(name, speed);
        }
        throw std::out_of_range("Speed should > 0");
    }

    void parseHikers(const YAML::Node& node, vector<Hiker>& hikers) {
        for (auto& hiker : node) {
            try {
                hikers.emplace_back(parseHiker(hiker));
            }
            catch (const std::exception& e) {
                std::cerr << "Parse hiker error: " << e.what() << std::endl;
                throw;
            }
        }
    }

    void parseBridge(const YAML::Node& node, double& length, vector<Hiker>& additionalHikers) {
        length = node["length"].as<double>();
        if (length <= 0) {
            throw std::out_of_range("Bridge's length should > 0");
        }
        parseHikers(node["hikers"], additionalHikers);
    }
};

void hikers_to_stream(std::ostringstream& oss, const vector<Hiker>& hikers)
{
    for (auto& hiker : hikers) {
        oss << "," << hiker.getName() << " " << hiker.getSpeed();
    }
}

string to_string_case(const vector<Hiker>& origHikers, const vector<Bridge>& bridges)
{
    std::ostringstream oss;
    hikers_to_stream(oss, origHikers);
    for (auto& bridge : bridges) {
        oss << ";" << bridge.getLength();
        hikers_to_stream(oss, bridge.getAdditionalHikers());
    }
    auto strCase = oss.str();
    return strCase.substr(1);
}

void run_yaml_case(const string& filename, bool verbose=false)
{
    vector<Hiker> origHikers;
    vector<Bridge> bridges;
    try {
        YAMLCaseParser parser;
        YAML::Node config = YAML::LoadFile(filename);
        parser.parse(config, origHikers, bridges);
    }
    catch (const std::exception& e) {
        std::cerr << "Parse case error: " << e.what() << std::endl;
        return;
    }
    if (verbose) {
        std::cout << "Case (accumulated additional hikers):\n"
            << to_string_case(origHikers, bridges) << std::endl;
    }
    Cache cache;
    CrossingTimeCalculator calc(&cache);
    double totalTime = calc.calcCrossingTime(bridges, origHikers, verbose);
    std::cout << "Total crossing time is " << totalTime << " minute(s)" << std::endl;
}
#else
void run_yaml_case(const string& filename, bool verbose=false)
{
}
#endif // ENABLE_YAML

// Test code

vector<string> split(const string& str, char delimiter)
{
    vector<string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));

    return tokens;
}

double parse_double(const string& str)
{
    double value = 0.0;
    try {
        value = std::stod(str);
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        throw;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Out of range: " << e.what() << std::endl;
        throw;
    }
    return value;
}

class CaseParser {
public:
    // String case format: <original hikers>;<bridge 1>;<bridge 2>;...;<bridge n>
    // Original hikers: name1 speed1, ..., name-n speed-n
    // Bridge: length, additionalHiker1 speed1, ..., additionalHiker-n speed-n (hikers are optional)
    // Example: "A 100,B 50,C 20,D 10;100;250,E 2.5;150,F 25,G 15"
    void parse(const string& strCase, vector<Hiker>& origHikers, vector<Bridge>& bridges) {
        auto items = split(strCase, ';');
        if (items.size() < 2) { // No bridge
            throw std::invalid_argument("Case format error: No bridge");
        }
        parseHikers(items[0], origHikers);
        sort_hikers(origHikers);
        vector<Hiker> additionalHikers;
        for (auto i = 1; i < items.size(); ++i) {
            double length = 0.0;
            size_t size = additionalHikers.size();
            parseBridge(items[i], length, additionalHikers);
            // New additional hiker(s) are added, need sort.
            if (additionalHikers.size() > size) {
                sort_hikers(additionalHikers);
            }
            bridges.emplace_back(Bridge(length, additionalHikers));
        }
    }

protected:
    // Hiker: name speed
    Hiker parseHiker(const string& strHiker) {
        auto hiker = split(strHiker, ' ');
        if (hiker.size() < 2) {
            throw std::invalid_argument("Hiker format error: " + strHiker);
        }
        auto speed = parse_double(hiker[1]);
        if (speed > 0) {
            return Hiker(hiker[0], speed);
        }
        throw std::out_of_range("Speed should > 0");
    }

    // Hikers: name1 speed1, ..., name-n speed-n
    void parseHikers(const string& strHikers, vector<Hiker>& hikers) {
        for (auto& strHiker : split(strHikers, ',')) {
            try {
                hikers.emplace_back(parseHiker(strHiker));
            }
            catch (const std::exception& e) {
                std::cerr << "Parse hiker error: " << e.what() << std::endl;
                throw;
            }
        }
    }

    // Bridge: length, additionalHiker1 speed1, ..., additionalHiker-n speed-n (hikers are optional)
    void parseBridge(const string& strBridge, double& length, vector<Hiker>& additionalHikers) {
        auto items = split(strBridge, ',');
        if (items.empty()) {
            throw std::invalid_argument("Bridge format error: No bridge length. " + strBridge);
        }
        length = parse_double(items[0]);
        if (length <= 0) {
            throw std::out_of_range("Bridge's length should > 0");
        }
        for (auto i = 1; i < items.size(); ++i) {
            additionalHikers.emplace_back(parseHiker(items[i]));
        }
    }
};

double run_case(const string& strCase, bool verbose=false)
{
    vector<Hiker> origHikers;
    vector<Bridge> bridges;
    try {
        CaseParser parser;
        parser.parse(strCase, origHikers, bridges);
    }
    catch (const std::exception& e) {
        std::cerr << "Parse case error: " << e.what() << std::endl;
        return -1.0;
    }
    if (verbose) {
        std::cout << "Case: " << strCase << std::endl;
    }
    Cache cache;
    CrossingTimeCalculator calc(&cache);
    double totalTime = calc.calcCrossingTime(bridges, origHikers, verbose);
    if (verbose) {
        std::cout << totalTime << " minute(s)" << std::endl;
    }
    return totalTime;
}

bool double_equal(double lhs, double rhs) {
    double epsilon = 1e-10; // std::numeric_limits<double>::epsilon();
    return std::fabs(lhs - rhs) < epsilon;
}

struct TestCase {
    double time;
    string strCase;
};

void run_tests()
{
    bool verbose = true; // TODO: make it a parameter
    TestCase cases[] = {
        {245, "A 100,B 50,C 20,D 10;100;250,E 2.5;150,F 25,G 15"},
        {1, "A 100;100"},
        {2, "A 100,B 50;100"},
        {17, "A 100,B 50,C 20,D 10;100"}, // C and D together is the fastest.
        {21, "A 100,B 25,C 20,D 10;100"}, // A and C, A and D is the fastest.
        {51, "A 100,B 50,C 20,D 10;100;200"}, // Cache hit.
        {55,   "A 100,B 50,C 20,D 10;100;200,E 200"},
        {55.5, "A 100,B 50,C 20,D 10;100;200,E 80"},
        {57,   "A 100,B 50,C 20,D 10;100;200,E 50"},
        {63, "A 100;100,B 50,C 20,D 10;200,E 50"},
    };
    for (auto& testCase : cases) {
        double time = run_case(testCase.strCase, verbose);
        bool pass = double_equal(time, testCase.time);
        std::cerr << (pass ? "P" : " F") << std::endl;
        if (!pass) {
            std::cerr << "Expected " << testCase.time << ", was " << time << std::endl;
        }
    }
}

int main(int argc, const char* argv[])
{
    if (argc > 1) {
        string yamlFile = argv[1];
        run_yaml_case(yamlFile, true);
    }
    else {
        run_tests();
    }
    return 0;
}
