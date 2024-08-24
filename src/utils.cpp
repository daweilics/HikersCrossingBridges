#include "utils.h"

#include <cmath>
#include <algorithm>
#include <iostream>
#include "hiker.h"
#include "bridge.h"


using std::string;
using std::vector;

// Sort hikers by speed in descending order.
void sort_hikers(vector<Hiker>& hikers)
{
    std::sort(hikers.begin(), hikers.end(), [](const Hiker& lhs, const Hiker& rhs) {
        return lhs.getSpeed() > rhs.getSpeed();
    });
}

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

bool double_equal(double lhs, double rhs)
{
    double epsilon = 1e-10; // std::numeric_limits<double>::epsilon();
    return std::fabs(lhs - rhs) < epsilon;
}
