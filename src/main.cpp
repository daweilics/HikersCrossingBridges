#include <string>
#include <vector>
#include <iostream>
#include <yaml-cpp/yaml.h>

#include "hiker.h"
#include "bridge.h"
#include "cache.h"
#include "calculator.h"
#include "yaml_parser.h"
#include "string_parser.h"
#include "utils.h"


using std::string;
using std::vector;

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

// Test code

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

struct TestCase {
    double time;
    string strCase;
};

void run_tests()
{
    bool verbose = true;
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
