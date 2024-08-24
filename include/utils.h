#pragma once
#include <string>
#include <vector>
#include <sstream>


class Hiker;
class Bridge;

// Sort hikers by speed in descending order.
void sort_hikers(std::vector<Hiker>& hikers);

void hikers_to_stream(std::ostringstream& oss, const std::vector<Hiker>& hikers);

std::string to_string_case(const std::vector<Hiker>& origHikers,
    const std::vector<Bridge>& bridges);

std::vector<std::string> split(const std::string& str, char delimiter);

double parse_double(const std::string& str);

bool double_equal(double lhs, double rhs);
